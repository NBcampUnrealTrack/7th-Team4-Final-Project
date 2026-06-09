// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTPlayerStatusWidget.generated.h"

class UPTHealthBarwidget;
class UPTManaBarWidget;
class UPTExpBarWidget;
class APTPlayerCharacter;
class APTBaseCharacter;     // 캐릭터 클래스
class APlayerController;
class APawn;

UCLASS()
class PENTAGRAM_API UPTPlayerStatusWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // 디버그 갱신
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Status|Debug")
    void DebugSetAll(float Hp, float MaxHp, float Mp, float MaxMp, float Exp, float ReqExp);

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    // 폰 바인딩
    void TryBindFromOwningPawn();

    // 바인딩
    void BindToCharacter(APTBaseCharacter* InCharacter);
    void UnbindFromCharacter();

    // 폰 변경
    UFUNCTION()
    void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

    void RefreshStatsUntilValid();

public:
    // 최초 1회
    bool bInitialStatsApplied = false;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTHealthBarwidget> HealthBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTManaBarWidget> ManaBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTExpBarWidget> ExpBar;

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<APTBaseCharacter> BoundCharacter;

    UPROPERTY(Transient)
    TWeakObjectPtr<APlayerController> BoundPC;
};
