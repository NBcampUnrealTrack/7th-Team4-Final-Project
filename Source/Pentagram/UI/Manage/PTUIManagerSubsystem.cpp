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
    // 진입 로그
    UE_LOG(LogTemp, Warning, TEXT("[OpenUILevel] In=[%s]"), *LevelName.ToString());

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
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] Failed to load widget class for level %s."), *LevelName.ToString());
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
