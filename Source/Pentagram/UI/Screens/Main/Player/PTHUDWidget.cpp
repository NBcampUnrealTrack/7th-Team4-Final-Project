// Fill out your copyright notice in the Description page of Project Settings.

<<<<<<< HEAD:Source/Pentagram/UI/Screens/Main/PTHUDWidget.cpp
=======

>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion"):Source/Pentagram/UI/Screens/Main/Player/PTHUDWidget.cpp
#include "UI/Screens/Main/Player/PTHUDWidget.h"
#include "Input/CommonUIInputTypes.h"

UPTHUDWidget::UPTHUDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bAutoActivate  = true;
    bIsBackHandler = false;
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
