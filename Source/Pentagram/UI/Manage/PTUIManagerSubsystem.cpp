

#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Screens/PTHUDWidget.h"
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

    // 베이스 레이아웃 등록
    PrimaryLayout = InLayout;
}

UPTHUDWidget* UPTUIManagerSubsystem::PushWidget(TSubclassOf<UPTHUDWidget> WidgetClass, EPTUILayer Layer)
{
    if (!WidgetClass || !PrimaryLayout.IsValid()) return nullptr;

    UCommonActivatableWidgetStack* Stack = PrimaryLayout->GetLayerStack(Layer);
    if (!Stack) return nullptr;

    // CommonUI 스택에 위젯 추가 (포커스 자동 전환)
    UCommonActivatableWidget* Added = Stack->AddWidget(WidgetClass);
    return Cast<UPTHUDWidget>(Added);
}

void UPTUIManagerSubsystem::RemoveWidget(UPTHUDWidget* WidgetToRemove)
{
    if (!WidgetToRemove) return;

    // 위젯 종료 (스택 및 화면에서 자동 제거)
    WidgetToRemove->DeactivateWidget();
}
