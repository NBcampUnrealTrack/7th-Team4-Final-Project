// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/PTPrimaryLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

UCommonActivatableWidgetStack* UPTPrimaryLayout::GetLayerStack(EPTUILayer Layer) const
{
    switch (Layer)
    {
    case EPTUILayer::HUD:      return GameLayer;
    case EPTUILayer::GameMenu: return MenuLayer;
    case EPTUILayer::Modal:    return ModalLayer;
    default:                   return nullptr;
    }
}
