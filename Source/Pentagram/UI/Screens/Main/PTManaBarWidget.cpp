// Fill out your copyright notice in the Description page of Project Settings.


#include "PTManaBarWidget.h"
#include "Components/ProgressBar.h"

void UPTManaBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::TopToBottom);
    }
}
void UPTManaBarWidget::HandleManaChanged(float Current, float Max)
{
    SetValue(Current, Max);
}
