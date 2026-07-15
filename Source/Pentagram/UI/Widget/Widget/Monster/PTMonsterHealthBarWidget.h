#pragma once

#include "CoreMinimal.h"
#include "../Player/PTStatBarWidget.h"
#include "PTMonsterHealthBarWidget.generated.h"

class APTMonsterCharacter;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTMonsterHealthBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    // 타겟 설정
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void ActivateForMonster(APTMonsterCharacter* InMonster);

    // 타겟 해제
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void ClearTarget();

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 바인딩
    void BindToMonster(APTMonsterCharacter* Monster);
    void UnbindFromMonster(APTMonsterCharacter* Monster);

    // 숨김 타이머
    void StartHideTimer();
    void ClearHideTimer();

    UFUNCTION()
    void HandleHideTimeout();

    // HP 콜백
    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

    // 이름 표시
    void UpdateMonsterName(APTMonsterCharacter* Monster);

protected:
    // 숨김 지연
    UPROPERTY(EditAnywhere, Category = "PT|UI|Monster")
    float HideDelay = 5.f;

    // 몬스터 이름 텍스트
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Name;

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<APTMonsterCharacter> BoundMonster;

    FTimerHandle HideTimerHandle;
};
