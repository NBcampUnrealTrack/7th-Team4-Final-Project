// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTPlayerStatusWidget.generated.h"


class UPTHealthBarwidget;
class UPTManaBarWidget;
class UPTExpBarWidget;
class APTBaseCharacter;     // ← 팀원의 캐릭터 클래스. 실제 이름이 다르면 변경
class APlayerController;
class APawn;

UCLASS()
class PENTAGRAM_API UPTPlayerStatusWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    /** 테스트/디버그용: 캐릭터 없이도 값 강제 갱신 */
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Status|Debug")
    void DebugSetAll(float Hp, float MaxHp, float Mp, float MaxMp, float Exp, float ReqExp);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTHealthBarwidget> HealthBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTManaBarWidget> ManaBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTExpBarWidget> ExpBar;

private:
    /** 현재 빙의된 폰에서 캐릭터를 찾아 바인딩 시도 */
    void TryBindFromOwningPawn();

    /** 실제 바인딩/언바인딩 */
    void BindToCharacter(APTBaseCharacter* InCharacter);
    void UnbindFromCharacter();

    /** 빙의된 폰이 바뀔 때 호출 (위젯이 떴는데 폰이 아직 없거나 교체된 경우) */
    UFUNCTION()
    void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

    UPROPERTY(Transient)
    TWeakObjectPtr<APTBaseCharacter> BoundCharacter;

    UPROPERTY(Transient)
    TWeakObjectPtr<APlayerController> BoundPC;
};
