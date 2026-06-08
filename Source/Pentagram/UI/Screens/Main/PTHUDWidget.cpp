// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screens/Main/PTHUDWidget.h"
#include "Input/CommonUIInputTypes.h"

UPTHUDWidget::UPTHUDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bAutoActivate  = true;
    bIsBackHandler = false;
}

TOptional<FUIInputConfig> UPTHUDWidget::GetDesiredInputConfig() const
{
    return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture);
}

void UPTHUDWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
}

void UPTHUDWidget::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
}

bool UPTHUDWidget::NativeOnHandleBackAction()
{
    return false;
}
