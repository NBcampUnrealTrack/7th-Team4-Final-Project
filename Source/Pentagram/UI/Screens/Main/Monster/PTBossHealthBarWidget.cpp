// PTBossHealthBarWidget.cpp
#include "PTBossHealthBarWidget.h"
<<<<<<< HEAD
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

=======

#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Components/TextBlock.h"

>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
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

<<<<<<< HEAD
    // 초기 동기화
=======
    // 초기 페이즈 강제 동기화 (CurrentPhase == INDEX_NONE이므로 반드시 1회 실행)
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
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

<<<<<<< HEAD
    // 페이즈 재계산
=======
    // HP 변경 시 페이즈 재계산 (보스가 OnPhaseChanged를 브로드캐스트하지 않아도 동작).
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
    if (APTBossMonsterCharacter* Boss = BoundBoss.Get())
    {
        HandlePhaseChanged(Boss->GetCurrentPhase());
    }
}
<<<<<<< HEAD
=======

void UPTBossHealthBarWidget::HandlePhaseChanged(int32 NewPhase)
{
    if (NewPhase == CurrentPhase)
    {
        return;
    }

    CurrentPhase = NewPhase;
    OnBossPhaseChanged(NewPhase);
}

void UPTBossHealthBarWidget::SetBossName(const FText& InName)
{
    if (Txt_BossName)
    {
        Txt_BossName->SetText(InName);
    }
}
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
