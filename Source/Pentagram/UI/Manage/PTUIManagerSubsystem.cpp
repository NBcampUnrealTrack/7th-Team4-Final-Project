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
#include "Interface/PTUIContentBoundsInterface.h"
#include "UI/Widget/Notify/PTNotifyManagerWidget.h"
#include "UI/Widget/Notify/PTNotifyTypes.h"

UPTUIManagerSubsystem::UPTUIManagerSubsystem()
{
}

void UPTUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UPTUIManagerSubsystem::Deinitialize()
{
    CloseAllGameplayUI();
    RemoveWidget(CurrentUIWidget);
    CurrentUIWidget = nullptr;

    if (CurrentNotifyWidget)
    {
        CurrentNotifyWidget->RemoveFromParent();
        CurrentNotifyWidget = nullptr;
    }

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
        return nullptr;
    }

    if (!PrimaryLayout.IsValid())
    {
        return nullptr;
    }

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
    WidgetToRemove->DeactivateWidget();
}

void UPTUIManagerSubsystem::CloseAllGameplayUI()
{
    RemoveWidget(InventoryInstance);
    InventoryInstance = nullptr;
    RemoveWidget(ShopInstance);
    ShopInstance = nullptr;
    CloseShopInventory();
    RemoveWidget(QuestInstance);
    QuestInstance = nullptr;
    RemoveWidget(SkillWindowInstance);
    SkillWindowInstance = nullptr;
    RemoveWidget(CharacterSheetInstance);
    CharacterSheetInstance = nullptr;
}

void UPTUIManagerSubsystem::OpenUILevel(FName LevelName)
{
    // 진입 로그
    UE_LOG(LogTemp, Warning, TEXT("[OpenUILevel] In=[%s]"), *LevelName.ToString());

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

    bAllowGameplayUI = Entry->bAllowGameplayUI;
    if (!bAllowGameplayUI)
    {
        CloseAllGameplayUI();
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
        UE_LOG(LogTemp, Warning, TEXT("[UI] Failed to load widget class for level %s."), *LevelName.ToString());
        UE_LOG(LogTemp, Warning, TEXT("[UI] OpenUILevel failed. WidgetClass is null. Level=%s Path=%s"),
            *LevelName.ToString(), *Entry->WidgetClass.ToSoftObjectPath().ToString());
    }

    SetupNotifyWidgetForLevel(*Entry);

    if (!Entry->ZoneDisplayName.IsEmpty())
    {
        FPTNotifyData ZoneData;
        ZoneData.Type = EPTNotifyType::ZoneEnter;
        ZoneData.Message = Entry->ZoneDisplayName;

        ShowNotify(ZoneData);
    }
}

void UPTUIManagerSubsystem::SetupNotifyWidgetForLevel(const FPTUILevelEntry& InEntry)
{
    if (CurrentNotifyWidget)
    {
        CurrentNotifyWidget->RemoveFromParent();
        CurrentNotifyWidget = nullptr;
    }

    if (InEntry.NotifyWidgetClass.IsNull())
    {
        return;
    }

    UClass* NotifyClass = InEntry.NotifyWidgetClass.LoadSynchronous();
    if (!NotifyClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] Failed to load notify widget class."));
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    UWorld* World = GetWorld();
    APlayerController* PlayerController =
        LocalPlayer != nullptr && World != nullptr ? LocalPlayer->GetPlayerController(World) : nullptr;
    if (!PlayerController)
    {
        return;
    }

    CurrentNotifyWidget = CreateWidget<UPTNotifyManagerWidget>(PlayerController, NotifyClass);
    if (CurrentNotifyWidget)
    {
        CurrentNotifyWidget->AddToPlayerScreen(100);
    }
}

void UPTUIManagerSubsystem::ShowNotify(const FPTNotifyData& InData)
{
    if (!CurrentNotifyWidget)
    {
        return;
    }

    CurrentNotifyWidget->Enqueue(InData);
}

void UPTUIManagerSubsystem::ToggleInventory(TSubclassOf<UCommonActivatableWidget> InventoryClass)
{
    if (!InventoryClass) return;
    if (!CanOpenGameplayUI()) return;

    const bool bWasOpen = InventoryInstance &&
        (InventoryInstance->IsActivated() || InventoryInstance->IsInViewport());

    CloseAllGameplayUI();

    if (bWasOpen)
    {
        return;
    }

    InventoryInstance = PushWidget(InventoryClass, EPTUILayer::GameMenu);
}

void UPTUIManagerSubsystem::ToggleShop(TSubclassOf<UCommonActivatableWidget> ShopClass)
{
    if (!ShopClass) return;
    if (!CanOpenGameplayUI()) return;

    const bool bWasOpen = ShopInstance &&
        (ShopInstance->IsActivated() || ShopInstance->IsInViewport());

    CloseAllGameplayUI();

    if (bWasOpen)
    {
        return;
    }

    ShopInstance = PushWidget(ShopClass, EPTUILayer::GameMenu);
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
    if (!CanOpenGameplayUI())
    {
        return;
    }

    const bool bWasOpen = QuestInstance &&
        (QuestInstance->IsActivated() || QuestInstance->IsInViewport());

    CloseAllGameplayUI();

    if (bWasOpen)
    {
        return;
    }

    QuestInstance = Cast<UPTNPCDialogueWidget>(
        PushWidget(QuestClass, EPTUILayer::GameMenu));
    if (QuestInstance != nullptr)
    {
        QuestInstance->SetupQuestJournal();
    }
}

void UPTUIManagerSubsystem::ToggleSkillWindow(TSubclassOf<UCommonActivatableWidget> SkillWindowClass)
{
    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 5. ToggleSkillWindow 진입"));

    if (!SkillWindowClass)
    {
        return;
    }

    if (!CanOpenGameplayUI())
    {
        return;
    }

    const bool bWasOpen = SkillWindowInstance &&
        (SkillWindowInstance->IsActivated() || SkillWindowInstance->IsInViewport());

    CloseAllGameplayUI();

    if (bWasOpen)
    {
        return;
    }

    SkillWindowInstance = PushWidget(SkillWindowClass, EPTUILayer::GameMenu);

    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 9. PushWidget 결과=%d"), SkillWindowInstance != nullptr);
}

void UPTUIManagerSubsystem::ToggleCharacterSheet(TSubclassOf<UCommonActivatableWidget> CharacterSheetClass)
{
    if (!CharacterSheetClass)
    {
        return;
    }

    if (!CanOpenGameplayUI())
    {
        return;
    }

    const bool bWasOpen = CharacterSheetInstance &&
        (CharacterSheetInstance->IsActivated() || CharacterSheetInstance->IsInViewport());

    CloseAllGameplayUI();

    if (bWasOpen)
    {
        return;
    }

    CharacterSheetInstance = PushWidget(CharacterSheetClass, EPTUILayer::GameMenu);
}

bool UPTUIManagerSubsystem::IsScreenPositionOverGameplayUI(const FVector2D& ScreenPosition) const
{
    auto IsOverWidget = [&ScreenPosition](UCommonActivatableWidget* Widget) -> bool
    {
        if (!Widget)
        {
            return false;
        }

        if (!Widget->IsActivated() && !Widget->IsInViewport())
        {
            return false;
        }

        if (Widget->Implements<UPTUIContentBoundsInterface>())
        {
            return IPTUIContentBoundsInterface::Execute_IsScreenPositionOverContent(Widget, ScreenPosition);
        }

        return false;
    };

    return IsOverWidget(InventoryInstance)
        || IsOverWidget(ShopInstance)
        || IsOverWidget(ShopInventoryInstance)
        || IsOverWidget(QuestInstance)
        || IsOverWidget(SkillWindowInstance)
        || IsOverWidget(CharacterSheetInstance);
}
