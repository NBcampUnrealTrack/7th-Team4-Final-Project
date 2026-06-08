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
    /** 보스 이름 라벨 설정(선택). */
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Boss")
    void SetBossName(const FText& InName);

    /** OnPhaseChanged 델리게이트 콜백(보스가 명시적으로 브로드캐스트할 경우). */
    UFUNCTION()
    void HandlePhaseChanged(int32 NewPhase);

protected:
    virtual void OnMonsterBound(APTMonsterCharacter* InMonster) override;
    virtual void OnMonsterUnbound(APTMonsterCharacter* InMonster) override;
    virtual void OnHealthChangedNative(float Current, float Max) override;

    /** BP에서 페이즈 전환 연출(바 색상, 이펙트, 사운드 등)을 구현. */
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|Boss")
    void OnBossPhaseChanged(int32 NewPhase);

    /** 선택적 보스 이름 라벨. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_BossName;

    /** 현재 페이즈 캐시. INDEX_NONE으로 시작해 최초 동기화 시 반드시 1회 갱신. */
    UPROPERTY(BlueprintReadOnly, Category = "PT|UI|Boss")
    int32 CurrentPhase = INDEX_NONE;

private:
    TWeakObjectPtr<APTBossMonsterCharacter> BoundBoss;
};
