#pragma once

#include "CoreMinimal.h"
#include "../Player/PTStatBarWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTMonsterHealthBarWidget.generated.h"

class APTMonsterCharacter;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTMonsterHealthBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    // 몬스터 바인딩
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void SetupMonster(APTMonsterCharacter* InMonster);

    // 타겟 표시
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void ActivateForMonster(APTMonsterCharacter* InMonster, float HideAfterSeconds = 5.0f);

    // HP 콜백
    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

    // 이름 바인딩
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Monster")
    void SetDisplayName(const FText& InName);

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 바인딩 직후
    virtual void OnMonsterBound(APTMonsterCharacter* InMonster) {}

    // 해제 직전
    virtual void OnMonsterUnbound(APTMonsterCharacter* InMonster) {}

    // HP 적용 후
    virtual void OnHealthChangedNative(float Current, float Max) {}

    // 자동 숨김
    void HandleAutoHide();

    // 바인딩 대상
    UPROPERTY(Transient)
    TWeakObjectPtr<APTMonsterCharacter> BoundMonster;

    // 몬스터 이름
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Name;

    // 숨김 타이머
    FTimerHandle AutoHideTimerHandle;

    // 자동 숨김 여부
    bool bUseAutoHide = false;
};
