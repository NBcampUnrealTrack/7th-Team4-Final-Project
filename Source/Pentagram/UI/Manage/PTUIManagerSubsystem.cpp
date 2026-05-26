
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Widget/PTPrimaryLayout.h"
#include "UI/Widget/PTActivatableWidgetBase.h"
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
    PrimaryLayout.Reset();
    Super::Deinitialize();
}

void UPTUIManagerSubsystem::RegisterPrimaryLayout(UPTPrimaryLayout* InLayout)
{
    if (!InLayout)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTUI] RegisterPrimaryLayout: InLayout is null"));
        return;
    }
    PrimaryLayout = InLayout;
    UE_LOG(LogTemp, Log, TEXT("[PTUI] PrimaryLayout registered"));
}

UPTActivatableWidgetBase* UPTUIManagerSubsystem::PushWidget(TSubclassOf<UPTActivatableWidgetBase> WidgetClass, EPTUILayer Layer)
{
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTUI] PushWidget: WidgetClass is null"));
        return nullptr;
    }

    if (!PrimaryLayout.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTUI] PushWidget: PrimaryLayout not registered. Call RegisterPrimaryLayout first (from APTHUD)."));
        return nullptr;
    }

    UCommonActivatableWidgetStack* Stack = PrimaryLayout->GetLayerStack(Layer);
    if (!Stack)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTUI] PushWidget: no stack for layer %d"), static_cast<int32>(Layer));
        return nullptr;
    }

    // CommonUI 스택에 push — 위젯 생성/추가/활성화/입력 라우팅 모두 자동 처리
    UCommonActivatableWidget* Added = Stack->AddWidget(WidgetClass);
    UPTActivatableWidgetBase* Result = Cast<UPTActivatableWidgetBase>(Added);

    if (!Result)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTUI] PushWidget: AddWidget returned null or wrong type for %s"), *WidgetClass->GetName());
        return nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("[PTUI] Pushed %s on layer %d"),
        *WidgetClass->GetName(), static_cast<int32>(Layer));

    return Result;
}

void UPTUIManagerSubsystem::RemoveWidget(UPTActivatableWidgetBase* WidgetToRemove)
{
    if (!WidgetToRemove) return;

    // CommonActivatableWidget은 DeactivateWidget으로 스택에서 자동 pop
    WidgetToRemove->DeactivateWidget();
}
