// Fill out your copyright notice in the Description page of Project Settings.

#include "PTExpBarWidget.h"
#include "Components/ProgressBar.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTExpBarWidget::HandleExpChanged(float Current, float Required)
{
    SetValue(Current, Required);
}

void UPTExpBarWidget::HandleLevelChanged(int32 NewLevel)
{
    // 레벨업 초기화
    SetValueInstant(0.f, MaxValue);
    OnLevelUpVisual(NewLevel);
}

void UPTExpBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::LeftToRight);
    }
}

void UPTExpBarWidget::BindToPlayerState(APTBasePlayerState* PS)
{
    PS->OnExpChanged.AddUniqueDynamic(this, &UPTExpBarWidget::HandleExpChanged);
    PS->OnLevelChanged.AddUniqueDynamic(this, &UPTExpBarWidget::HandleLevelChanged);
}

void UPTExpBarWidget::UnbindFromPlayerState(APTBasePlayerState* PS)
{
    PS->OnExpChanged.RemoveDynamic(this, &UPTExpBarWidget::HandleExpChanged);
    PS->OnLevelChanged.RemoveDynamic(this, &UPTExpBarWidget::HandleLevelChanged);
}
