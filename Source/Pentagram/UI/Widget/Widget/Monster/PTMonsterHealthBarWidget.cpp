#include "PTMonsterHealthBarWidget.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UPTMonsterHealthBarWidget::SetupMonster(APTMonsterCharacter* InMonster)
{
    if (!InMonster)
    {
        return;
    }

    if (APTMonsterCharacter* Old = BoundMonster.Get())
    {
        if (Old == InMonster)
        {
            SetValueInstant(InMonster->CurrentHP, InMonster->MaxHP);
            return;
        }

        OnMonsterUnbound(Old);
        Old->OnHPChanged.RemoveDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);
    }

    BoundMonster = InMonster;
    InMonster->OnHPChanged.AddUniqueDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);

    OnMonsterBound(InMonster);

    SetValueInstant(InMonster->CurrentHP, InMonster->MaxHP);
}

void UPTMonsterHealthBarWidget::ActivateForMonster(APTMonsterCharacter* InMonster, float HideAfterSeconds)
{
    if (!InMonster)
    {
        return;
    }

    SetupMonster(InMonster);

    // 클릭 통과
    SetVisibility(ESlateVisibility::HitTestInvisible);

    bUseAutoHide = HideAfterSeconds > 0.0f;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoHideTimerHandle);

        if (bUseAutoHide)
        {
            World->GetTimerManager().SetTimer(
                AutoHideTimerHandle, this,
                &UPTMonsterHealthBarWidget::HandleAutoHide,
                HideAfterSeconds, false);
        }
    }
}

void UPTMonsterHealthBarWidget::HandleHealthChanged(float Current, float Max)
{
    SetValue(Current, Max);
    OnHealthChangedNative(Current, Max);

    // 사망 시 숨김
    if (bUseAutoHide && Current <= 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                AutoHideTimerHandle, this,
                &UPTMonsterHealthBarWidget::HandleAutoHide,
                1.0f, false);
        }
    }
}

void UPTMonsterHealthBarWidget::HandleAutoHide()
{
    SetVisibility(ESlateVisibility::Collapsed);

    if (APTMonsterCharacter* Monster = BoundMonster.Get())
    {
        OnMonsterUnbound(Monster);
        Monster->OnHPChanged.RemoveDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);
    }
    BoundMonster.Reset();
}

void UPTMonsterHealthBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::LeftToRight);
    }
}

void UPTMonsterHealthBarWidget::SetDisplayName(const FText& InName)
{
    if (Txt_Name)
    {
        Txt_Name->SetText(InName);
    }
}

void UPTMonsterHealthBarWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
    }

    if (APTMonsterCharacter* Monster = BoundMonster.Get())
    {
        OnMonsterUnbound(Monster);
        Monster->OnHPChanged.RemoveDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);
    }
    BoundMonster.Reset();

    Super::NativeDestruct();
}
