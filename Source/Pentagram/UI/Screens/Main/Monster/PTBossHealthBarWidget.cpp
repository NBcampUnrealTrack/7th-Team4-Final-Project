// PTBossHealthBarWidget.cpp
#include "PTBossHealthBarWidget.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Components/TextBlock.h"

void UPTBossHealthBarWidget::SetBossName(const FText& InName)
{
    if (Txt_BossName)
    {
        Txt_BossName->SetText(InName);
    }
}

void UPTBossHealthBarWidget::HandlePhaseChanged(int32 NewPhase)
{
    if (NewPhase == CurrentPhase)
    {
        return;
    }

    CurrentPhase = NewPhase;
    OnBossPhaseChanged(NewPhase);
}

void UPTBossHealthBarWidget::OnMonsterBound(APTMonsterCharacter* InMonster)
{
    Super::OnMonsterBound(InMonster);

    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(InMonster);
    if (!Boss)
    {
        return;
    }

    BoundBoss = Boss;
    Boss->OnPhaseChanged.AddUniqueDynamic(this, &UPTBossHealthBarWidget::HandlePhaseChanged);

    // 초기 동기화
    HandlePhaseChanged(Boss->GetCurrentPhase());
}

void UPTBossHealthBarWidget::OnMonsterUnbound(APTMonsterCharacter* InMonster)
{
    if (APTBossMonsterCharacter* Boss = BoundBoss.Get())
    {
        Boss->OnPhaseChanged.RemoveDynamic(this, &UPTBossHealthBarWidget::HandlePhaseChanged);
    }
    BoundBoss.Reset();

    Super::OnMonsterUnbound(InMonster);
}

void UPTBossHealthBarWidget::OnHealthChangedNative(float Current, float Max)
{
    Super::OnHealthChangedNative(Current, Max);

    // 페이즈 재계산
    if (APTBossMonsterCharacter* Boss = BoundBoss.Get())
    {
        HandlePhaseChanged(Boss->GetCurrentPhase());
    }
}
