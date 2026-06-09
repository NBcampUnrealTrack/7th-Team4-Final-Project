// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Screens/Main/PTHUDWidget.h"
#include "UI/Screens/LayOut/PTPrimaryLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

UPTUIManagerSubsystem::UPTUIManagerSubsystem()
{
}

void UPTUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UPTUIManagerSubsystem::Deinitialize()
{
    // 참조 해제
    PrimaryLayout.Reset();
    Super::Deinitialize();
}

void UPTUIManagerSubsystem::RegisterPrimaryLayout(UPTPrimaryLayout* InLayout)
{
    if (!InLayout) return;

    UE_LOG(LogTemp, Warning, TEXT(">> Register: Subsystem=%p, InLayout=%p"), this, InLayout);

    // 레이아웃 등록
    PrimaryLayout = InLayout;
}

UCommonActivatableWidget* UPTUIManagerSubsystem::PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass,
    EPTUILayer Layer)
{
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PushWidget: WidgetClass가 null"));
        return nullptr;
    }
    if (!PrimaryLayout.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("PushWidget: PrimaryLayout.가 null"));
        return nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT(">> PushWidget: Layer=%d, Subsystem=%p, PrimaryLayout=%p"),
        (int32)Layer, this, PrimaryLayout.Get());

    UCommonActivatableWidgetStack* Stack = PrimaryLayout->GetLayerStack(Layer);
    if (!Stack)
    {
        UE_LOG(LogTemp, Warning, TEXT("PushWidget: Stack가 null"));
        return nullptr;
    }
    return Stack->AddWidget(WidgetClass);
}

void UPTUIManagerSubsystem::RemoveWidget(UCommonActivatableWidget* WidgetToRemove)
{
    if (!WidgetToRemove) return;

    // 위젯 종료
    WidgetToRemove->DeactivateWidget();
}

void UPTUIManagerSubsystem::ToggleInventory(TSubclassOf<UCommonActivatableWidget> InventoryClass)
{

    if (InventoryInstance && InventoryInstance->IsActivated())
    {
        InventoryInstance->DeactivateWidget();
        return;
    }

    // 열림 판정
    if (InventoryInstance)
    {
        if (InventoryInstance->IsActivated() || InventoryInstance->IsInViewport())
        {
            bIsInventoryOpen = true;
        }
    }

    if (bIsInventoryOpen)
    {
        RemoveWidget(InventoryInstance);
        InventoryInstance = nullptr;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT(">> 인벤토리 Push 실패!"));
    }
}
