#include "PTMonsterHealthBarWidget.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Components/ProgressBar.h"

void UPTMonsterHealthBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::LeftToRight);
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
            // 동일 몬스터: 값만 즉시 동기화하고 종료
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
