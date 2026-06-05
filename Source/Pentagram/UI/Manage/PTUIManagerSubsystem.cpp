

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
    // 참조 안전 해제
    PrimaryLayout.Reset();
    Super::Deinitialize();
}

void UPTUIManagerSubsystem::RegisterPrimaryLayout(UPTPrimaryLayout* InLayout)
{
    if (!InLayout) return;
    UE_LOG(LogTemp, Warning, TEXT(">> Register: Subsystem=%p, InLayout=%p"), this, InLayout);
    // 베이스 레이아웃 등록
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
    if(!PrimaryLayout.IsValid())
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

    // 위젯 종료 (스택 및 화면에서 자동 제거)
    WidgetToRemove->DeactivateWidget();
}

void UPTUIManagerSubsystem::ToggleInventory(TSubclassOf<UCommonActivatableWidget> InventoryClass)
{

    if (InventoryInstance && InventoryInstance->IsActivated())
    {
        InventoryInstance->DeactivateWidget();
        return;
    }

    // 2. 인벤토리가 없거나, 닫혀 있는 경우 (열기)
    // 인벤토리가 닫혀있다면(IsValid()가 false이거나 Deactivated 상태) 새로 Push
    InventoryInstance = PushWidget(InventoryClass, EPTUILayer::GameMenu);

    // [중요] 생성된 후 바로 활성화해주어야 Common UI가 입력을 받습니다.
    if (InventoryInstance)
    {
        InventoryInstance->ActivateWidget();
        UE_LOG(LogTemp, Warning, TEXT(">> 인벤토리 활성화 완료!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT(">> 인벤토리 Push 실패!"));
    }
}
