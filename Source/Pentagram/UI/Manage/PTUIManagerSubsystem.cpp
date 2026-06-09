// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Screens/Main/Player/PTHUDWidget.h"
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
        return nullptr;
    }
    if (!PrimaryLayout.IsValid())
    {
        return nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT(">> PushWidget: Layer=%d, Subsystem=%p, PrimaryLayout=%p"),
        (int32)Layer, this, PrimaryLayout.Get());

    UCommonActivatableWidgetStack* Stack = PrimaryLayout->GetLayerStack(Layer);
    if (!Stack)
    {
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
    if (!InventoryClass) return;

    bool bIsInventoryOpen = false;

<<<<<<< HEAD
    // 열림 판정
=======
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
    if (InventoryInstance)
    {
        if (InventoryInstance->IsActivated() || InventoryInstance->IsInViewport())
        {
            bIsInventoryOpen = true;
        }
    }
<<<<<<< HEAD

=======
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
    if (bIsInventoryOpen)
    {
        RemoveWidget(InventoryInstance);
        InventoryInstance = nullptr;
<<<<<<< HEAD
=======

>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
    }
    else
    {
        InventoryInstance = PushWidget(InventoryClass, EPTUILayer::GameMenu);
    }


}
