// Fill out your copyright notice in the Description page of Project Settings.

#include "PTManaBarWidget.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTManaBarWidget::HandleManaChanged(float Current, float Max)
{
    SetValue(Current, Max);
}

void UPTManaBarWidget::BindToPlayerState(APTBasePlayerState* PS)
{
    PS->OnManaChanged.AddUniqueDynamic(this, &UPTManaBarWidget::HandleManaChanged);
}

void UPTManaBarWidget::UnbindFromPlayerState(APTBasePlayerState* PS)
{
    PS->OnManaChanged.RemoveDynamic(this, &UPTManaBarWidget::HandleManaChanged);
}
