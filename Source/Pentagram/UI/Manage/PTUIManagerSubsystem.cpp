#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/HUD/PTHUDWidget.h"
#include "UI/Widget/LayOut/PTPrimaryLayout.h"
#include "UI/Widget/NPC/PTNPCDialogueWidget.h"
#include "UI/Widget/Shop/PTShopWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "UI/Setting/PTUISettings.h"

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
    if (!InLayout) return;
    PrimaryLayout = InLayout;
}

UCommonActivatableWidget* UPTUIManagerSubsystem::PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass,
    EPTUILayer Layer)
{
    if (!WidgetClass) return nullptr;
    if (!PrimaryLayout.IsValid()) return nullptr;

    UCommonActivatableWidgetStack* Stack = PrimaryLayout->GetLayerStack(Layer);
    if (!Stack) return nullptr;

    return Stack->AddWidget(WidgetClass);
}

void UPTUIManagerSubsystem::RemoveWidget(UCommonActivatableWidget* WidgetToRemove)
{
    if (!WidgetToRemove) return;
    WidgetToRemove->DeactivateWidget();
}

void UPTUIManagerSubsystem::OpenUILevel(FName LevelName)
{
    // 표 조회
    const UPTUISettings* Settings = GetDefault<UPTUISettings>();
    if (!Settings) return;

    const FPTUILevelEntry* Entry = Settings->LevelUITable.Find(LevelName);
    if (!Entry) return;

    UWorld* World = GetLocalPlayer() ? GetLocalPlayer()->GetWorld() : nullptr;
    if (!World) return;

    // 이전 UI 제거
    if (CurrentUIWidget)
    {
        RemoveWidget(CurrentUIWidget);
        CurrentUIWidget = nullptr;
    }

    // 이전 스트림 언로드
    if (!CurrentStreamLevelName.IsNone() && CurrentStreamLevelName != LevelName)
    {
        FLatentActionInfo UnloadInfo;
        UnloadInfo.UUID = 2;
        UGameplayStatics::UnloadStreamLevel(World, CurrentStreamLevelName, UnloadInfo, false);
        CurrentStreamLevelName = NAME_None;
    }

    // 새 스트림 로드
    if (Entry->bIsStreamingLevel)
    {
        FLatentActionInfo LoadInfo;
        LoadInfo.UUID = 1;
        UGameplayStatics::LoadStreamLevel(World, LevelName, true, true, LoadInfo);
        CurrentStreamLevelName = LevelName;
    }

    // 매핑된 UI 푸시
    if (UClass* WidgetClass = Entry->WidgetClass.LoadSynchronous())
    {
        CurrentUIWidget = PushWidget(WidgetClass, Entry->Layer);
    }
}

void UPTUIManagerSubsystem::ToggleInventory(TSubclassOf<UCommonActivatableWidget> InventoryClass)
{
    if (!InventoryClass) return;

    bool bIsInventoryOpen = false;
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
        InventoryInstance = PushWidget(InventoryClass, EPTUILayer::GameMenu);
    }
}

void UPTUIManagerSubsystem::ToggleShop(TSubclassOf<UCommonActivatableWidget> ShopClass)
{
    if (!ShopClass) return;

    bool bIsShopOpen = false;
    if (ShopInstance)
    {
        if (ShopInstance->IsActivated() || ShopInstance->IsInViewport())
        {
            bIsShopOpen = true;
        }
    }

    if (bIsShopOpen)
    {
        RemoveWidget(ShopInstance);
        ShopInstance = nullptr;
    }
    else
    {
        ShopInstance = PushWidget(ShopClass, EPTUILayer::GameMenu);
    }
}

void UPTUIManagerSubsystem::ToggleQuest(TSubclassOf<UPTNPCDialogueWidget> QuestClass)
{
    if (QuestClass == nullptr)
    {
        return;
    }

    if (QuestInstance != nullptr &&
        (QuestInstance->IsActivated() || QuestInstance->IsInViewport()))
    {
        RemoveWidget(QuestInstance);
        QuestInstance = nullptr;
        return;
    }

    QuestInstance = Cast<UPTNPCDialogueWidget>(
        PushWidget(QuestClass, EPTUILayer::GameMenu));
    if (QuestInstance != nullptr)
    {
        QuestInstance->SetupQuestJournal();
    }
}
