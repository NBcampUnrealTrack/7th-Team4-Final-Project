#include "PTMonsterHealthBarWidget.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UPTMonsterHealthBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 최초 숨김
    SetVisibility(ESlateVisibility::Collapsed);
}

void UPTMonsterHealthBarWidget::NativeDestruct()
{
    // 잔여 정리
    ClearTarget();
    Super::NativeDestruct();
}

void UPTMonsterHealthBarWidget::ActivateForMonster(APTMonsterCharacter* InMonster)
{
    if (!InMonster)
    {
        return;
    }
    // 같은 타겟이면 표시/타이머만 갱신
    if (BoundMonster.Get() == InMonster)
    {
        SetVisibility(ESlateVisibility::HitTestInvisible);
        StartHideTimer();
        return;
    }

    // 새 타겟 → 기존 정리
    ClearTarget();

    BoundMonster = InMonster;
    BindToMonster(InMonster);

    // 이름 표시
    UpdateMonsterName(InMonster);

    // 현재 HP 즉시 반영 후 표시
    SetValueInstant(InMonster->CurrentHP, InMonster->MaxHP);
    SetVisibility(ESlateVisibility::HitTestInvisible);

    StartHideTimer();
}

void UPTMonsterHealthBarWidget::ClearTarget()
{
    // 타이머 정리
    ClearHideTimer();

    // 화면 숨김
    SetVisibility(ESlateVisibility::Collapsed);

    // 바인딩 해제
    if (APTMonsterCharacter* OldMonster = BoundMonster.Get())
    {
        UnbindFromMonster(OldMonster);
    }
    BoundMonster.Reset();
}

void UPTMonsterHealthBarWidget::BindToMonster(APTMonsterCharacter* Monster)
{
    if (Monster)
    {
        Monster->OnHPChanged.AddUniqueDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);
    }
}

void UPTMonsterHealthBarWidget::UnbindFromMonster(APTMonsterCharacter* Monster)
{
    if (Monster)
    {
        Monster->OnHPChanged.RemoveDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);
    }
}

void UPTMonsterHealthBarWidget::StartHideTimer()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // 공격마다 리셋
    World->GetTimerManager().SetTimer(
        HideTimerHandle, this, &UPTMonsterHealthBarWidget::HandleHideTimeout, HideDelay, false);

}

void UPTMonsterHealthBarWidget::ClearHideTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(HideTimerHandle);
    }
}

void UPTMonsterHealthBarWidget::HandleHideTimeout()
{
    UE_LOG(LogTemp, Log, TEXT("[MonsterHP] Hide by timeout"));
    ClearTarget();
}

void UPTMonsterHealthBarWidget::HandleHealthChanged(float Current, float Max)
{
    UE_LOG(LogTemp, Log, TEXT("[MonsterHP] HP %.0f/%.0f"), Current, Max);

    // 보간 반영
    SetValue(Current, Max);

    // 사망 즉시 숨김
    if (Current <= 0.0f)
    {
        ClearTarget();
    }
}

void UPTMonsterHealthBarWidget::UpdateMonsterName(APTMonsterCharacter* Monster)
{
    if (!Txt_Name || !Monster)
    {
        return;
    }

    Txt_Name->SetText(Monster->GetMonsterDisplayName());
}
