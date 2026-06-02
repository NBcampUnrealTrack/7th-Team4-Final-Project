// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screens/LayOut/PTPrimaryLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

UCommonActivatableWidgetStack* UPTPrimaryLayout::GetLayerStack(EPTUILayer Layer) const
{
    UE_LOG(LogTemp, Warning, TEXT("GetLayerStack 호출: Layer=%d / GameLayer=%s, MenuLayer=%s, ModalLayer=%s"),
        (int32)Layer,
        GameLayer ? TEXT("OK") : TEXT("NULL"),
        MenuLayer ? TEXT("OK") : TEXT("NULL"),
        ModalLayer ? TEXT("OK") : TEXT("NULL"));
    switch (Layer)
    {
    case EPTUILayer::HUD:      return GameLayer;
    case EPTUILayer::GameMenu: return MenuLayer;
    case EPTUILayer::Modal:    return ModalLayer;
    default:                   return nullptr;
    }
}
