#include "UI/Manage/PTUIManagerSubsystem.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/PlayerController.h"
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
    CloseShopInventory();
    RemoveWidget(InventoryInstance);
    InventoryInstance = nullptr;
    RemoveWidget(ShopInstance);
    ShopInstance = nullptr;
    RemoveWidget(QuestInstance);
    QuestInstance = nullptr;
    RemoveWidget(CurrentUIWidget);
    CurrentUIWidget = nullptr;
    PrimaryLayout.Reset();
    Super::Deinitialize();
}

void UPTUIManagerSubsystem::RegisterPrimaryLayout(UPTPrimaryLayout* InLayout)
{
    if (!InLayout)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] RegisterPrimaryLayout failed. Layout is null."));
        return;
    }

    PrimaryLayout = InLayout;
    UE_LOG(LogTemp, Log, TEXT("[UI] Primary layout registered: %s"), *GetNameSafe(InLayout));
}

UCommonActivatableWidget* UPTUIManagerSubsystem::PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass,
    EPTUILayer Layer)
{
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] PushWidget failed. WidgetClass is null."));
        return nullptr;
    }

    if (!PrimaryLayout.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] PushWidget failed for %s. PrimaryLayout is not registered."),
            *GetNameSafe(WidgetClass.Get()));
        return nullptr;
    }

    UCommonActivatableWidgetStack* Stack = PrimaryLayout->GetLayerStack(Layer);
    if (!Stack)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] PushWidget failed for %s. Layer stack is null. Layer=%d"),
            *GetNameSafe(WidgetClass.Get()), static_cast<int32>(Layer));
        return nullptr;
    }

    UCommonActivatableWidget* AddedWidget = Stack->AddWidget(WidgetClass);
    UE_LOG(LogTemp, Log, TEXT("[UI] PushWidget %s to Layer=%d Result=%s"),
        *GetNameSafe(WidgetClass.Get()), static_cast<int32>(Layer), *GetNameSafe(AddedWidget));
    return AddedWidget;
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
    if (!Settings)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] OpenUILevel failed. PTUISettings is null. Level=%s"),
            *LevelName.ToString());
        return;
    }

    const FPTUILevelEntry* Entry = Settings->LevelUITable.Find(LevelName);
    if (!Entry)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] OpenUILevel failed. LevelUITable has no entry for %s."),
            *LevelName.ToString());
        return;
    }

    UWorld* World = GetLocalPlayer() ? GetLocalPlayer()->GetWorld() : nullptr;
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] OpenUILevel failed. World is null. Level=%s"),
            *LevelName.ToString());
        return;
    }

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
    UClass* WidgetClass = Entry->WidgetClass.Get();
    if (WidgetClass == nullptr && !Entry->WidgetClass.IsNull())
    {
        WidgetClass = Entry->WidgetClass.LoadSynchronous();
    }

    if (WidgetClass)
    {
        CurrentUIWidget = PushWidget(WidgetClass, Entry->Layer);
        if (CurrentUIWidget == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("[UI] OpenUILevel failed to push widget. Level=%s Widget=%s Layer=%d"),
                *LevelName.ToString(), *GetNameSafe(WidgetClass), static_cast<int32>(Entry->Layer));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] OpenUILevel failed. WidgetClass is null. Level=%s Path=%s"),
            *LevelName.ToString(), *Entry->WidgetClass.ToSoftObjectPath().ToString());
    }
}

void UPTUIManagerSubsystem::ToggleInventory(TSubclassOf<UCommonActivatableWidget> InventoryClass)
{
    if (!InventoryClass) return;

    CloseShopInventory();

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
        CloseShopInventory();
    }
    else
    {
        ShopInstance = PushWidget(ShopClass, EPTUILayer::GameMenu);
    }
}

UCommonActivatableWidget* UPTUIManagerSubsystem::OpenInventoryForShop(
    TSubclassOf<UCommonActivatableWidget> InventoryClass)
{
    if (!InventoryClass)
    {
        return nullptr;
    }

    if (ShopInventoryInstance &&
        (ShopInventoryInstance->IsActivated() || ShopInventoryInstance->IsInViewport()))
    {
        return ShopInventoryInstance;
    }

    if (InventoryInstance)
    {
        RemoveWidget(InventoryInstance);
        InventoryInstance = nullptr;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    UWorld* World = GetWorld();
    APlayerController* PlayerController =
        LocalPlayer != nullptr && World != nullptr ? LocalPlayer->GetPlayerController(World) : nullptr;
    if (PlayerController == nullptr)
    {
        return nullptr;
    }

    ShopInventoryInstance = CreateWidget<UCommonActivatableWidget>(PlayerController, InventoryClass);
    if (ShopInventoryInstance == nullptr)
    {
        return nullptr;
    }

    ShopInventoryInstance->AddToPlayerScreen(20);
    ShopInventoryInstance->ActivateWidget();
    return ShopInventoryInstance;
}

void UPTUIManagerSubsystem::CloseShopInventory()
{
    if (ShopInventoryInstance == nullptr)
    {
        return;
    }

    ShopInventoryInstance->DeactivateWidget();
    ShopInventoryInstance->RemoveFromParent();
    ShopInventoryInstance = nullptr;
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
