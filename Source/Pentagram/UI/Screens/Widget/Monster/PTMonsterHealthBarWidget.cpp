#include "PTMonsterHealthBarWidget.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

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

void UPTMonsterHealthBarWidget::HandleHealthChanged(float Current, float Max)
{
    SetValue(Current, Max);
    OnHealthChangedNative(Current, Max);
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
    if (APTMonsterCharacter* Monster = BoundMonster.Get())
    {
        OnMonsterUnbound(Monster);
        Monster->OnHPChanged.RemoveDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);
    }
    BoundMonster.Reset();

    Super::NativeDestruct();
}
