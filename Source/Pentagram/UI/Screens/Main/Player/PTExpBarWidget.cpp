// Fill out your copyright notice in the Description page of Project Settings.


#include "PTExpBarWidget.h"
#include "Components/ProgressBar.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTExpBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::LeftToRight);
    }
}

void UPTExpBarWidget::HandleExpChanged(float Current, float Required)
{
    SetValue(Current, Required);
}

void UPTExpBarWidget::HandleLevelChanged(int32 NewLevel)
{
    // 레벨업 순간 바를 0으로 초기화(즉시) — Current는 다음 ExpChanged broadcast에서 옴
    SetValueInstant(0.f, MaxValue);
    OnLevelUpVisual(NewLevel);
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
