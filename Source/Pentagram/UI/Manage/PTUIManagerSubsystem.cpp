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
#include "UI/Widget/Notify/PTNotifyManagerWidget.h" // 실제 경로에 맞게 수정
#include "UI/Widget/Notify/PTNotifyTypes.h"                   // 실제 경로에 맞게 수정

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
    RemoveWidget(SkillWindowInstance);
    SkillWindowInstance = nullptr;

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

    // 이 레벨에서 게임플레이 UI(인벤토리/샵/퀘스트/스킬창) 허용 여부 갱신
    bAllowGameplayUI = Entry->bAllowGameplayUI;
    if (!bAllowGameplayUI)
    {
        // 레벨 전환 시 열려있던 게임플레이 UI 강제로 닫기
        RemoveWidget(InventoryInstance);
        InventoryInstance = nullptr;
        RemoveWidget(ShopInstance);
        ShopInstance = nullptr;
        CloseShopInventory();
        RemoveWidget(QuestInstance);
        QuestInstance = nullptr;
        RemoveWidget(SkillWindowInstance);
        SkillWindowInstance = nullptr;
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

    // 레벨별 알림 위젯 갱신 (None이면 내부에서 스킵)
    SetupNotifyWidgetForLevel(*Entry);

    // 지역명이 설정돼 있으면 레벨 진입과 동시에 자동 표시
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
    // 레벨이 바뀌면 기존 알림 위젯은 항상 정리
    if (CurrentNotifyWidget)
    {
        CurrentNotifyWidget->RemoveFromParent();
        CurrentNotifyWidget = nullptr;
    }

    // 이 레벨은 알림 기능 없음 (인트로, 메인메뉴 등)
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
        // 다른 UI(샵 등 ZOrder 20)보다 위에 뜨도록 높게 설정
        CurrentNotifyWidget->AddToPlayerScreen(100);
    }
}

void UPTUIManagerSubsystem::ShowNotify(const FPTNotifyData& InData)
{
    if (!CurrentNotifyWidget)
    {
        // 현재 레벨에 알림 위젯이 설정 안 돼있음 (None) - 조용히 무시
        return;
    }

    CurrentNotifyWidget->Enqueue(InData);
}

void UPTUIManagerSubsystem::ToggleInventory(TSubclassOf<UCommonActivatableWidget> InventoryClass)
{
    if (!InventoryClass) return;
    if (!CanOpenGameplayUI()) return;

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
    if (!CanOpenGameplayUI()) return;

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
    if (!CanOpenGameplayUI())
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

void UPTUIManagerSubsystem::ToggleSkillWindow(TSubclassOf<UCommonActivatableWidget> SkillWindowClass)
{
    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 5. ToggleSkillWindow 진입"));

    if (!SkillWindowClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[SkillWindow] 6. SkillWindowClass가 NULL이라 리턴"));
        return;
    }

    if (!CanOpenGameplayUI())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 6-1. 현재 레벨은 게임플레이 UI 비허용이라 리턴"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 6. PrimaryLayout Valid=%d"), PrimaryLayout.IsValid());

    bool bIsOpen = false;
    if (SkillWindowInstance)
    {
        if (SkillWindowInstance->IsActivated() || SkillWindowInstance->IsInViewport())
        {
            bIsOpen = true;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 7. bIsOpen=%d (SkillWindowInstance=%d)"),
        bIsOpen, SkillWindowInstance != nullptr);

    if (bIsOpen)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 8. 이미 열려있어서 닫음"));
        RemoveWidget(SkillWindowInstance);
        SkillWindowInstance = nullptr;
        return;
    }

    // 인벤토리/샵과 겹치지 않게 정리
    if (InventoryInstance) { RemoveWidget(InventoryInstance); InventoryInstance = nullptr; }
    if (ShopInstance) { RemoveWidget(ShopInstance); ShopInstance = nullptr; CloseShopInventory(); }

    SkillWindowInstance = PushWidget(SkillWindowClass, EPTUILayer::GameMenu);

    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 9. PushWidget 결과=%d"), SkillWindowInstance != nullptr);
}
