// Fill out your copyright notice in the Description page of Project Settings.


#include "PTMonsterHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTMonsterHealthBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::LeftToRight);
    }
}

void UPTMonsterHealthBarWidget::HandleHealthChanged(float Current, float Max)
{
    UE_LOG(LogTemp, Warning, TEXT("HandleHealthChanged %f / %f"), Current, Max);
    SetValue(Current, Max);
}

void UPTMonsterHealthBarWidget::BindToPlayerState(APTBasePlayerState* PS)
{
    PS->OnHealthChanged.AddUniqueDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);
}
void UPTMonsterHealthBarWidget::UnbindFromPlayerState(APTBasePlayerState* PS)
{
    PS->OnHealthChanged.RemoveDynamic(this, &UPTMonsterHealthBarWidget::HandleHealthChanged);
}
