#pragma once

#include "CoreMinimal.h"
#include "PTMonsterHealthBarWidget.h"
#include "PTBossHealthBarWidget.generated.h"

class APTBossMonsterCharacter;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTBossHealthBarWidget : public UPTMonsterHealthBarWidget
{
    GENERATED_BODY()

public:
    // 보스명 설정
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Boss")
    void SetBossName(const FText& InName);

    // 페이즈 콜백
    UFUNCTION()
    void HandlePhaseChanged(int32 NewPhase);

protected:
    // 오버라이드
    virtual void OnMonsterBound(APTMonsterCharacter* InMonster) override;
    virtual void OnMonsterUnbound(APTMonsterCharacter* InMonster) override;
    virtual void OnHealthChangedNative(float Current, float Max) override;

    // 페이즈 연출
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|Boss")
    void OnBossPhaseChanged(int32 NewPhase);

    // 보스명 라벨
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_BossName;

    // 페이즈 캐시
    UPROPERTY(BlueprintReadOnly, Category = "PT|UI|Boss")
    int32 CurrentPhase = INDEX_NONE;

private:
    // 바인딩 보스
    TWeakObjectPtr<APTBossMonsterCharacter> BoundBoss;
};
