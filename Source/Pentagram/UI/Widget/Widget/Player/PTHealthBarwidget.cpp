// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/Widget/Player/PTHealthBarwidget.h"
#include "Components/ProgressBar.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTHealthBarwidget::HandleHealthChanged(float Current, float Max)
{
    UE_LOG(LogTemp, Warning, TEXT("HandleHealthChanged %f / %f"), Current, Max);
    SetValue(Current, Max);
}

void UPTHealthBarwidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::LeftToRight);
    }
}

void UPTHealthBarwidget::BindToPlayerState(APTBasePlayerState* PS)
{
    PS->OnHealthChanged.AddUniqueDynamic(this, &UPTHealthBarwidget::HandleHealthChanged);
}

void UPTHealthBarwidget::UnbindFromPlayerState(APTBasePlayerState* PS)
{
    PS->OnHealthChanged.RemoveDynamic(this, &UPTHealthBarwidget::HandleHealthChanged);
}
