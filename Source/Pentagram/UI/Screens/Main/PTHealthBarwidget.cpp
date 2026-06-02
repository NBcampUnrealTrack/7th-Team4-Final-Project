// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Screens/Main/PTHealthBarwidget.h"
#include "Components/ProgressBar.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTHealthBarwidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::RightToLeft);
    }
}

void UPTHealthBarwidget::HandleHealthChanged(float Current, float Max)
{
    SetValue(Current, Max);
}

void UPTHealthBarwidget::BindToPlayerState(APTBasePlayerState* PS)
{
    PS->OnHealthChanged.AddUniqueDynamic(this, &UPTHealthBarwidget::HandleHealthChanged);
}
void UPTHealthBarwidget::UnbindFromPlayerState(APTBasePlayerState* PS)
{
    PS->OnHealthChanged.RemoveDynamic(this, &UPTHealthBarwidget::HandleHealthChanged);
}
