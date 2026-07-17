#include "Character/Player/PTPlayerController.h"

#include "AudioMixerBlueprintLibrary.h"
#include "CommonActivatableWidget.h"
#include "Character/Player/PTBasePlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "PTEquipmentComponent.h"
#include "PTInventoryComponent.h"
#include "PTPlayerCharacter.h"
#include "UI/Widget/LayOut/PTPrimaryLayout.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Item/PTDropItemActorBase.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/NPC/PTNPCCharacter.h"
#include "Character/NPC/PTQuestNPCCharacter.h"
#include "Character/NPC/PTShopNPCCharacter.h"
#include "InteractionActor/PTInteractionActor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/PTGameMode.h"
#include "Core/Subsystems/PTEconomySubsystem.h"
#include "Core/Subsystems/PTItemSubsystem.h"
#include "Core/Subsystems/PTOnlineSubsystem.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/PTControlSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "Core/PTGameState.h" // 로비 채팅 추가
#include "UI/Widget/Inventory/PTInventoryWidget.h"
#include "UI/Widget/NPC/PTNPCDialogueWidget.h"
#include "UI/Widget/Shop/PTShopWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Framework/Application/SlateApplication.h"
#include "TimerManager.h"
#include "Character/Skill/PTSkillIndicatorActor.h"
#include "Materials/MaterialIREmitter.h"
#include "UObject/ConstructorHelpers.h"

APTPlayerController::APTPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;

    static ConstructorHelpers::FClassFinder<APTDropItemActorBase> DropItemBlueprint(
        TEXT("/Game/Pentagram/Item/BP_DropItem"));
    if (DropItemBlueprint.Succeeded())
    {
        InventoryDropActorClass = DropItemBlueprint.Class;
    }

    static ConstructorHelpers::FClassFinder<APTDropItemActorBase> PotionDropBlueprint(
        TEXT("/Game/Pentagram/Item/BP_DropItem_Potion"));
    static ConstructorHelpers::FClassFinder<APTDropItemActorBase> SkillBookDropBlueprint(
        TEXT("/Game/Pentagram/Item/BP_DropItem_SkillBook"));
    static ConstructorHelpers::FClassFinder<APTDropItemActorBase> ChestDropBlueprint(
        TEXT("/Game/Pentagram/Item/BP_DropItem_Chest"));
    static ConstructorHelpers::FClassFinder<APTDropItemActorBase> HelmetDropBlueprint(
        TEXT("/Game/Pentagram/Item/BP_DropItem_Helmet"));
    static ConstructorHelpers::FClassFinder<APTDropItemActorBase> WandDropBlueprint(
        TEXT("/Game/Pentagram/Item/BP_DropItem_Wand"));
    static ConstructorHelpers::FClassFinder<APTDropItemActorBase> ShovelDropBlueprint(
        TEXT("/Game/Pentagram/Item/BP_DropItem_Shovel"));

    PotionDropActorClass = PotionDropBlueprint.Class;
    SkillBookDropActorClass = SkillBookDropBlueprint.Class;
    ChestDropActorClass = ChestDropBlueprint.Class;
    HelmetDropActorClass = HelmetDropBlueprint.Class;
    WandDropActorClass = WandDropBlueprint.Class;
    ShovelDropActorClass = ShovelDropBlueprint.Class;
}

TSubclassOf<APTDropItemActorBase> APTPlayerController::ResolveInventoryDropActorClass(
    const FItemData& ItemData) const
{
    switch (ItemData.Item_Type)
    {
    case EItemType::Potion:
        return PotionDropActorClass != nullptr ? PotionDropActorClass : InventoryDropActorClass;
    case EItemType::SkillBook:
        return SkillBookDropActorClass != nullptr ? SkillBookDropActorClass : InventoryDropActorClass;
    case EItemType::Chest:
        return ChestDropActorClass != nullptr ? ChestDropActorClass : InventoryDropActorClass;
    case EItemType::Helmet:
        return HelmetDropActorClass != nullptr ? HelmetDropActorClass : InventoryDropActorClass;
    case EItemType::Weapon:
        if (ItemData.WeaponType == EWeaponType::Wand && WandDropActorClass != nullptr)
        {
            return WandDropActorClass;
        }
        if (ItemData.Item_ID.ToString().Contains(TEXT("Shovel"), ESearchCase::IgnoreCase) &&
            ShovelDropActorClass != nullptr)
        {
            return ShovelDropActorClass;
        }
        return InventoryDropActorClass;
    default:
        return InventoryDropActorClass;
    }
}

void APTPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalPlayerController()) return;

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UPTControlSettingsSubsystem* ControlSettingsSubsystem = GameInstance->GetSubsystem<UPTControlSettingsSubsystem>())
        {
            ControlSettingsSubsystem->ApplyControlSettings(this);
        }
    }

    AddUIInputMapping();

    if (PrimaryLayoutClass)
    {
        PrimaryLayout = CreateWidget<UPTPrimaryLayout>(this, PrimaryLayoutClass);
        if (PrimaryLayout)
        {
            PrimaryLayout->AddToPlayerScreen(0);
            if (ULocalPlayer* LP = GetLocalPlayer())
            {
                if (UPTUIManagerSubsystem* UIMgr = LP->GetSubsystem<UPTUIManagerSubsystem>())
                {
                    UIMgr->RegisterPrimaryLayout(PrimaryLayout);

                    // 기본값은 현재 맵 UI다. 온라인 로비 이동 직후에는 실제 맵(L_Intro)과
                    // 표시할 스트리밍 UI(L_Lobby)가 다르므로 서브시스템의 예약값을 우선한다.
                    FName UILevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
                    UGameInstance* CurrentGameInstance = GetGameInstance();
                    UPTOnlineSubsystem* OnlineSubsystem = CurrentGameInstance != nullptr
                        ? CurrentGameInstance->GetSubsystem<UPTOnlineSubsystem>()
                        : nullptr;
                    if (OnlineSubsystem != nullptr && OnlineSubsystem->TravelToPendingSessionJoin(this))
                    {
                        return;
                    }

                    if (OnlineSubsystem != nullptr)
                    {
                        const FName PendingUILevelName = OnlineSubsystem->ConsumePendingLocalUILevelName();
                        if (!PendingUILevelName.IsNone())
                        {
                            UILevelName = PendingUILevelName;
                        }
                    }

                    UIMgr->OpenUILevel(UILevelName);
                }
            }
        }
    }
}

bool APTPlayerController::CanMove(APTPlayerCharacter* PC) const
{
    if (!PC) return false;
    return !PC->bIsTransitioningToCombat
        && !PC->bIsStaggered
        && !PC->bIsDodging;
    // 공격 중 이동 취소를 허용하려면 bIsAttacking은 뺀다
}

void APTPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsAiming) UpdateSkillAim();   // 조준 중 매 프레임 갱신

    if (!bMoveToDestination) return;

    ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
    if (!MyCharacter) return;

    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(MyCharacter))
    {
        if (PC->bIsTransitioningToCombat)
        {
            bMoveToDestination = false;
            MyCharacter->GetCharacterMovement()->StopMovementImmediately();
            return;
        }
    }

    FVector Direction = MoveDestination - MyCharacter->GetActorLocation();
    Direction.Z = 0.f;

    if (Direction.Size2D() <= AcceptanceRadius)
    {
        bMoveToDestination = false;
        MyCharacter->GetCharacterMovement()->StopMovementImmediately();
        return;
    }

    MyCharacter->AddMovementInput(Direction.GetSafeNormal(), 1.f);
}

void APTPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    UE_LOG(LogTemp, Warning, TEXT("Controller SetupInputComponent Called"));

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (EnhancedInput)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller EnhancedInput Cast Success"));

        if (IA_Move)      EnhancedInput->BindAction(IA_Move,      ETriggerEvent::Triggered, this, &APTPlayerController::OnRightClick);
        if (IA_Attack)    EnhancedInput->BindAction(IA_Attack,    ETriggerEvent::Started,   this, &APTPlayerController::OnLeftClick);
        if (IA_Inventory) EnhancedInput->BindAction(IA_Inventory, ETriggerEvent::Started,   this, &APTPlayerController::OnInventoryPressed);
        if (IA_Shop)      EnhancedInput->BindAction(IA_Shop,      ETriggerEvent::Started,   this, &APTPlayerController::OnShopPressed);
        if (IA_Quest)     EnhancedInput->BindAction(IA_Quest,     ETriggerEvent::Started,   this, &APTPlayerController::OnQuestPressed);
        if (IA_OpenSettings) EnhancedInput->BindAction(IA_OpenSettings, ETriggerEvent::Started, this, &APTPlayerController::OnSettingsPressed);
        if (IA_Dodge)     EnhancedInput->BindAction(IA_Dodge,     ETriggerEvent::Started,   this, &APTPlayerController::OnDodge);

        if (IA_Skill1)
        {
            EnhancedInput->BindAction(IA_Skill1, ETriggerEvent::Started,   this, &APTPlayerController::OnSkill1);
            EnhancedInput->BindAction(IA_Skill1, ETriggerEvent::Completed, this, &APTPlayerController::OnSkill1Released);
        }
        if (IA_Skill2)
        {
            EnhancedInput->BindAction(IA_Skill2, ETriggerEvent::Started,   this, &APTPlayerController::OnSkill2);
            EnhancedInput->BindAction(IA_Skill2, ETriggerEvent::Completed, this, &APTPlayerController::OnSkill2Released);
        }
        if (IA_Skill3)
        {
            EnhancedInput->BindAction(IA_Skill3, ETriggerEvent::Started,   this, &APTPlayerController::OnSkill3);
            EnhancedInput->BindAction(IA_Skill3, ETriggerEvent::Completed, this, &APTPlayerController::OnSkill3Released);
        }
        if (IA_Skill4)
        {
            EnhancedInput->BindAction(IA_Skill4, ETriggerEvent::Started,   this, &APTPlayerController::OnSkill4);
            EnhancedInput->BindAction(IA_Skill4, ETriggerEvent::Completed, this, &APTPlayerController::OnSkill4Released);
        }
        if (IA_SkillWindow)  EnhancedInput->BindAction(IA_SkillWindow,  ETriggerEvent::Started,   this, &APTPlayerController::OnSkillWindowPressed);
        if (IA_CharacterSheet) EnhancedInput->BindAction(IA_CharacterSheet, ETriggerEvent::Started, this, &APTPlayerController::OnCharacterSheetPressed);
        if (IA_CloseMenu) EnhancedInput->BindAction(IA_CloseMenu, ETriggerEvent::Started, this, &APTPlayerController::OnCloseMenuPressed);
        // [디버그] 즉사
        // [디버그] 즉사
        if (IA_DebugKill) EnhancedInput->BindAction(IA_DebugKill, ETriggerEvent::Started,   this, &APTPlayerController::OnDebugKillPressed);

        if (IA_Interact)
        {
            UE_LOG(LogTemp, Warning, TEXT("IA_Interact Binding (F Key)"));
            EnhancedInput->BindAction(IA_Interact, ETriggerEvent::Started, this, &APTPlayerController::OnInteractPressed);
        }

        if (IA_QuickSlot1)
        {
            EnhancedInput->BindAction(IA_QuickSlot1, ETriggerEvent::Started, this, &APTPlayerController::OnQuickSlot1);
        }

        if (IA_QuickSlot2)
        {
            EnhancedInput->BindAction(IA_QuickSlot2, ETriggerEvent::Started, this, &APTPlayerController::OnQuickSlot2);
        }
    }

    AddUIInputMapping();
}

void APTPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (IMC_Default)
        {
            Subsystem->AddMappingContext(IMC_Default, 0);
        }
    }
    SetGameplayInputBlockedByUI(false);
    RestoreGameplayInput();
}

void APTPlayerController::Client_OpenQuestDialogue_Implementation(
    APTQuestNPCCharacter* QuestNPC,
    TSubclassOf<UPTNPCDialogueWidget> QuestDialogueWidgetClass)
{
    if (!IsLocalPlayerController() || QuestNPC == nullptr)
    {
        return;
    }

    if (QuestDialogueWidgetClass == nullptr)
    {
        QuestDialogueWidgetClass = QuestNPC->GetQuestDialogueWidgetClass();
    }

    if (QuestDialogueWidgetClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest dialogue widget class is null."));
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (LocalPlayer == nullptr)
    {
        return;
    }

    UPTUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPTUIManagerSubsystem>();
    if (UIManager == nullptr)
    {
        return;
    }

    UPTNPCDialogueWidget* DialogueWidget = Cast<UPTNPCDialogueWidget>(
        UIManager->PushWidget(QuestDialogueWidgetClass, EPTUILayer::GameMenu));
    if (DialogueWidget != nullptr)
    {
        DialogueWidget->SetupDialogue(QuestNPC);
    }
}

void APTPlayerController::Client_OpenShop_Implementation(
    APTShopNPCCharacter* ShopNPC,
    TSubclassOf<UPTShopWidget> ShopWidgetClass)
{
    if (!IsLocalPlayerController() || ShopNPC == nullptr)
    {
        return;
    }

    TSubclassOf<UCommonActivatableWidget> WidgetClass = ShopWidgetClass;
    if (WidgetClass == nullptr)
    {
        WidgetClass = ShopClass;
    }

    if (WidgetClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] Shop widget class is not configured."));
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (LocalPlayer == nullptr)
    {
        return;
    }

    UPTUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPTUIManagerSubsystem>();
    if (UIManager == nullptr)
    {
        return;
    }

    UPTShopWidget* ShopWidget = Cast<UPTShopWidget>(
        UIManager->PushWidget(WidgetClass, EPTUILayer::GameMenu));
    if (ShopWidget != nullptr)
    {
        ShopWidget->SetupShop(ShopNPC);

        if (InventoryClass != nullptr)
        {
            UPTInventoryWidget* InventoryWidget = Cast<UPTInventoryWidget>(
                UIManager->OpenInventoryForShop(InventoryClass));
            if (InventoryWidget != nullptr)
            {
                InventoryWidget->SetShopSellTarget(ShopWidget);
            }
        }
    }
    else
    {
        UIManager->CloseShopInventory();
    }
}

void APTPlayerController::ServerAcceptQuest_Implementation(APTQuestNPCCharacter* QuestNPC, FName QuestID)
{
    if (QuestNPC == nullptr || QuestID.IsNone() || !QuestNPC->GetQuestIDs().Contains(QuestID))
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UPTQuestSubsystem* QuestSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTQuestSubsystem>() : nullptr;
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->AcceptQuest(GetPlayerState<APTBasePlayerState>(), QuestID);
    }
}

bool APTPlayerController::ServerAcceptQuest_Validate(APTQuestNPCCharacter* QuestNPC, FName QuestID)
{
    return QuestNPC != nullptr && !QuestID.IsNone();
}

void APTPlayerController::ServerRewardQuest_Implementation(APTQuestNPCCharacter* QuestNPC, FName QuestID)
{
    if (QuestNPC == nullptr || QuestID.IsNone() || !QuestNPC->GetQuestIDs().Contains(QuestID))
    {
        return;
    }

    APTBasePlayerState* PTPlayerState = GetPlayerState<APTBasePlayerState>();
    if (PTPlayerState == nullptr)
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UPTQuestSubsystem* QuestSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTQuestSubsystem>() : nullptr;
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->RewardQuest(QuestID, PTPlayerState);
    }
}

bool APTPlayerController::ServerRewardQuest_Validate(APTQuestNPCCharacter* QuestNPC, FName QuestID)
{
    return QuestNPC != nullptr && !QuestID.IsNone();
}

void APTPlayerController::ServerBuyItem_Implementation(APTShopNPCCharacter* ShopNPC, FName ItemID)
{
    if (ShopNPC == nullptr || ItemID.IsNone() || !ShopNPC->GetProductIDs().Contains(ItemID))
    {
        return;
    }

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    APTBasePlayerState* PTPlayerState = GetPlayerState<APTBasePlayerState>();
    if (PlayerCharacter == nullptr || PTPlayerState == nullptr)
    {
        return;
    }

    const float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), ShopNPC->GetActorLocation());
    if (Distance > 350.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] Purchase rejected because the player is too far from the shop."));
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTItemSubsystem* ItemSubsystem = GameInstance->GetSubsystem<UPTItemSubsystem>();
    UPTEconomySubsystem* EconomySubsystem = GameInstance->GetSubsystem<UPTEconomySubsystem>();
    UPTInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent();
    if (ItemSubsystem == nullptr || EconomySubsystem == nullptr || InventoryComponent == nullptr)
    {
        return;
    }

    const FItemData* ItemData = ItemSubsystem->GetItemData(ItemID);
    if (ItemData == nullptr || ItemData->BuyPrice <= 0)
    {
        return;
    }

    if (!InventoryComponent->CanAddItem(*ItemData) ||
        !EconomySubsystem->CanAfford(PTPlayerState, ItemData->BuyPrice))
    {
        return;
    }

    if (!EconomySubsystem->SpendGold(PTPlayerState, ItemData->BuyPrice))
    {
        return;
    }

    if (!InventoryComponent->TryAddItem(*ItemData))
    {
        EconomySubsystem->AddGold(PTPlayerState, ItemData->BuyPrice);
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Shop] Purchased %s for %d gold."),
        *ItemData->Item_Name.ToString(),
        ItemData->BuyPrice);
}

bool APTPlayerController::ServerBuyItem_Validate(APTShopNPCCharacter* ShopNPC, FName ItemID)
{
    return ShopNPC != nullptr && !ItemID.IsNone();
}

void APTPlayerController::ServerSellItem_Implementation(
    APTShopNPCCharacter* ShopNPC,
    int32 InventorySlotIndex,
    FName ExpectedItemID,
    int32 Count)
{
    if (ShopNPC == nullptr || InventorySlotIndex < 0 || ExpectedItemID.IsNone() || Count <= 0)
    {
        return;
    }

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    APTBasePlayerState* PTPlayerState = GetPlayerState<APTBasePlayerState>();
    if (PlayerCharacter == nullptr || PTPlayerState == nullptr)
    {
        return;
    }

    const float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), ShopNPC->GetActorLocation());
    if (Distance > 350.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] Sale rejected because the player is too far from the shop."));
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTEconomySubsystem* EconomySubsystem = GameInstance->GetSubsystem<UPTEconomySubsystem>();
    UPTInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent();
    if (EconomySubsystem == nullptr || InventoryComponent == nullptr)
    {
        return;
    }

    const TArray<FInventorySlot>& InventorySlots = InventoryComponent->GetInventorySlots();
    if (!InventorySlots.IsValidIndex(InventorySlotIndex) ||
        InventorySlots[InventorySlotIndex].IsEmpty())
    {
        return;
    }

    const FInventorySlot SlotToSell = InventorySlots[InventorySlotIndex];
    if (SlotToSell.ItemData.Item_ID != ExpectedItemID)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] Sale rejected because slot item changed. Expected=%s Actual=%s"),
            *ExpectedItemID.ToString(),
            *SlotToSell.ItemData.Item_ID.ToString());
        return;
    }

    if (!SlotToSell.ItemData.CanSell())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shop] Item cannot be sold: %s"),
            *SlotToSell.ItemData.Item_Name.ToString());
        return;
    }

    const int32 SellCount = FMath::Clamp(Count, 1, SlotToSell.Quantity);
    const int32 UnitSellPrice = SlotToSell.ItemData.GetSellPrice();
    if (UnitSellPrice <= 0)
    {
        return;
    }

    if (!InventoryComponent->RemoveItemAtSlot(InventorySlotIndex, SellCount))
    {
        return;
    }

    const int32 TotalSellPrice = UnitSellPrice * SellCount;
    EconomySubsystem->AddGold(PTPlayerState, TotalSellPrice);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Shop] Sold %s x%d for %d gold."),
        *SlotToSell.ItemData.Item_Name.ToString(),
        SellCount,
        TotalSellPrice);
}

bool APTPlayerController::ServerSellItem_Validate(
    APTShopNPCCharacter* ShopNPC,
    int32 InventorySlotIndex,
    FName ExpectedItemID,
    int32 Count)
{
    return ShopNPC != nullptr && InventorySlotIndex >= 0 && !ExpectedItemID.IsNone() && Count > 0;
}

void APTPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveUIInputMapping();
    Super::EndPlay(EndPlayReason);
}

void APTPlayerController::RestoreGameplayInput()
{
    if (!IsLocalPlayerController())
    {
        return;
    }

    TWeakObjectPtr<APTPlayerController> WeakThis(this);
    GetWorldTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateLambda(
            [WeakThis]()
            {
                if (!WeakThis.IsValid())
                {
                    return;
                }

                APTPlayerController* PlayerController = WeakThis.Get();
                UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
                    PlayerController,
                    nullptr,
                    EMouseLockMode::DoNotLock,
                    false,
                    true);
                UWidgetBlueprintLibrary::SetFocusToGameViewport();
                PlayerController->SetShowMouseCursor(true);
            }));
}

void APTPlayerController::SetGameplayInputBlockedByUI(bool bBlocked)
{
    bGameplayInputBlockedByUI = bBlocked;

    if (!bGameplayInputBlockedByUI)
    {
        return;
    }

    bMoveToDestination = false;
    StopMovement();

    if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* MovementComponent = MyCharacter->GetCharacterMovement())
        {
            MovementComponent->StopMovementImmediately();
        }
    }
}


void APTPlayerController::Client_ShowDamageNumber_Implementation(FVector WorldLocation, float DamageAmount, bool bIsCritical)
{
    if (!DamageNumberWidgetClass) return;

    UPTDamageNumberWidget* Widget = CreateWidget<UPTDamageNumberWidget>(
        this, DamageNumberWidgetClass);
    if (!Widget) return;

    Widget->AddToViewport(10);
    Widget->InitDamageNumber(DamageAmount, bIsCritical);

    FVector2D ScreenPos;
    if (ProjectWorldLocationToScreen(WorldLocation, ScreenPos))
    {
        Widget->SetPositionInViewport(ScreenPos, false);
    }
}

void APTPlayerController::RequestEquipItem(int32 InventoryIndex, EItemType EquipType)
{
    UE_LOG(LogTemp, Warning, TEXT("[RequestEquipItem] Index=%d, HasAuthority=%d"), InventoryIndex, HasAuthority());

    if (!HasAuthority())
    {
        Server_RequestEquipItem(InventoryIndex, EquipType);
        return;
    }

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    UE_LOG(LogTemp, Warning, TEXT("[RequestEquipItem] PlayerCharacter=%d"), PlayerCharacter != nullptr);
    if (!PlayerCharacter) return;

    UPTInventoryComponent* Inven = PlayerCharacter->InventoryComponent;
    UPTEquipmentComponent* Equip = PlayerCharacter->EquipmentComponent;
    UE_LOG(LogTemp, Warning, TEXT("[RequestEquipItem] Inven=%d, Equip=%d"), Inven != nullptr, Equip != nullptr);
    if (!Inven || !Equip) return;

    // 슬롯 유효성 검사
    const TArray<FInventorySlot>& Slots = Inven->GetInventorySlots();
    UE_LOG(LogTemp, Warning, TEXT("[RequestEquipItem] SlotValid=%d, IsEmpty=%d"), Slots.IsValidIndex(InventoryIndex), Slots.IsValidIndex(InventoryIndex) ? Slots[InventoryIndex].IsEmpty() : true);
    if (!Slots.IsValidIndex(InventoryIndex)) return;

    const FInventorySlot& TargetSlot = Slots[InventoryIndex];
    if (TargetSlot.IsEmpty()) return;

    UE_LOG(LogTemp, Warning, TEXT("[RequestEquipItem] ItemType=%d, EquipType=%d"), (int32)TargetSlot.ItemData.Item_Type, (int32)EquipType);
    // 장비 타입 일치 검사 (드래그 검증 2차)
    if (TargetSlot.ItemData.Item_Type != EquipType) return;

    // 장착 요청 (내부에서 클라면 Server RPC 자동 호출)
    FItemData OldItem;
    Equip->EquipItem(TargetSlot.ItemData, OldItem);

    // 인벤토리에서 해당 슬롯 제거
    // 클라이언트에서는 낙관적 UI 업데이트 (서버 복제로 덮어씌워짐)
    Inven->RemoveItem(TargetSlot.ItemData.Item_ID, 1);

    // 교체된 구장비가 있으면 인벤토리에 돌려놓기
    if (!OldItem.Item_ID.IsNone())
    {
        Inven->TryAddItem(OldItem, 1);
    }
}

void APTPlayerController::RequestUnequipItem(EItemType EquipType, int32 ToInventoryIndex)
{
    if (!HasAuthority())
    {
        Server_RequestUnequipItem(EquipType);
        return;
    }

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;

    UPTInventoryComponent* Inven = PlayerCharacter->InventoryComponent;
    UPTEquipmentComponent* Equip = PlayerCharacter->EquipmentComponent;
    if (!Inven || !Equip) return;

    // EItemType → EEquipSlotType 변환
    EEquipSlotType SlotType = EEquipSlotType::Weapon;
    switch (EquipType)
    {
    case EItemType::Weapon: SlotType = EEquipSlotType::Weapon; break;
    case EItemType::Chest:  SlotType = EEquipSlotType::Chest;  break;
    case EItemType::Helmet: SlotType = EEquipSlotType::Helmet; break;
    case EItemType::Gloves: SlotType = EEquipSlotType::Gloves; break;
    case EItemType::Boots:  SlotType = EEquipSlotType::Boots;  break;
    default: return;
    }

    FItemData UnequippedItem;
    if (Equip->UnequipItem(SlotType, UnequippedItem))
    {
        if (!UnequippedItem.Item_ID.IsNone())
        {
            Inven->TryAddItem(UnequippedItem, 1);
        }
    }
}

void APTPlayerController::Server_RequestEquipItem_Implementation(int32 InventoryIndex, EItemType EquipType)
{
    RequestEquipItem(InventoryIndex, EquipType);
}

void APTPlayerController::Server_RequestUnequipItem_Implementation(EItemType EquipType)
{
    RequestUnequipItem(EquipType, 0);
}

void APTPlayerController::RefreshInventoryUI()
{
}

/*
void APTPlayerController::PlayAttackMontage()
{
    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;
    if (!PlayerCharacter->AttackMontages.IsValidIndex(PlayerCharacter->ComboIndex)) return;

    PlayerCharacter->bIsAttacking = true;
    PlayerCharacter->bCanCombo    = false;
    PlayerCharacter->PlayAnimMontage(PlayerCharacter->AttackMontages[PlayerCharacter->ComboIndex]);
    PlayerCharacter->Server_PlayAttackMontage(PlayerCharacter->ComboIndex);

    PlayerCharacter->ComboIndex++;
}*/

void APTPlayerController::RotateTowardsMouse()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;
    if (PC->bIsStaggered) return;

    FHitResult HitResult;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, HitResult)) return;
    if (!HitResult.bBlockingHit) return;

    FVector Direction = HitResult.Location - PC->GetActorLocation();
    Direction.Z = 0.f;
    if (Direction.IsNearlyZero()) return;

    FRotator Rotation = Direction.Rotation();
    PC->SetActorRotation(Rotation);
    Server_SetActorRotation(Rotation);
}

bool APTPlayerController::IsMouseOverGameplayUI() const
{
    if (!IsLocalPlayerController())
    {
        return false;
    }

    ULocalPlayer* LP = GetLocalPlayer();
    UPTUIManagerSubsystem* UIManager = LP != nullptr ? LP->GetSubsystem<UPTUIManagerSubsystem>() : nullptr;
    if (!UIManager)
    {
        return false;
    }

    // GetMousePosition()은 뷰포트 픽셀 좌표라서, GetCachedGeometry()/IsUnderLocation()이 쓰는
    // DPI 스케일 적용된 Slate 절대 좌표랑 어긋남 (DPI Scale이 1.0이 아니면 - 이 프로젝트는 0.67).
    // FSlateApplication의 커서 좌표를 써야 Slate 기준 좌표계랑 정확히 일치함.
    const FVector2D AbsoluteCursorPos = FSlateApplication::Get().GetCursorPos();

    return UIManager->IsScreenPositionOverGameplayUI(AbsoluteCursorPos);
}

void APTPlayerController::OnRightClick(const FInputActionValue& Value)
{
    if (bGameplayInputBlockedByUI) return;

    if (IsMouseOverGameplayUI())
    {
        // IA_Move는 Triggered라 누르고 있는 동안 매 프레임 호출됨.
        // 드래그 중 커서가 UI 위로 들어오면 새 목적지를 안 찍는 것만으론 부족하고,
        // 이미 진행 중이던 이동도 여기서 같이 끊어야 함 (안 그러면 Tick이 UI 진입 전
        // 목적지를 향해 계속 걸어감).
        bMoveToDestination = false;
        StopMovement();
        return;
    }

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (PC && !CanMove(PC)) return;

    if (PC->bIsDodging) return;

    if (PC->bIsTransitioningToCombat) return;

    if (PC->SkillComp->bIsAttacking)
    {
        PC->StopAnimMontage();
        PC->SkillComp->Server_StopBasicAttack();
    }

    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    if (!HitResult.bBlockingHit) return;

    MoveDestination = HitResult.Location;
    bMoveToDestination = true;
}

void APTPlayerController::OnLeftClick(const FInputActionValue& Value)
{
    if (bGameplayInputBlockedByUI) return;
    if (IsMouseOverGameplayUI()) return;

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;
    if (PlayerCharacter->bIsDodging) return;
    if (PlayerCharacter->bIsUsingSkill) return;

    bMoveToDestination = false;
    StopMovement();

    FHitResult HitResult;
    bool bGotHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    if (bGotHit && HitResult.bBlockingHit)
    {
        FVector Direction = HitResult.Location - PlayerCharacter->GetActorLocation();
        Direction.Z = 0.f;
        if (!Direction.IsNearlyZero())
        {
            if (!PlayerCharacter->bIsStaggered)
            {
                FRotator NewRotation = Direction.Rotation();
                PlayerCharacter->SetActorRotation(NewRotation);
                Server_SetActorRotation(NewRotation);
            }
        }

        APTDropItemActorBase* TargetItem = Cast<APTDropItemActorBase>(HitResult.GetActor());
        if (TargetItem)
        {
            float Distance2D = FVector::Dist2D(PlayerCharacter->GetActorLocation(), TargetItem->GetActorLocation());

            if (Distance2D <= 250.0f)
            {
                Server_TryPickupItem(TargetItem);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("아이템이 너무 멀리 있습니다."));
            }
            return;
        }
    }

    //무기 장착 여부 검사
    if (!PlayerCharacter->EquipmentComponent || !PlayerCharacter->EquipmentComponent->IsWeaponEquipped())
    {
        return;
    }

    PlayerCharacter->SkillComp->TryBasicAttack();
}

void APTPlayerController::OnInteractPressed()
{
    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (PlayerCharacter)
    {
        if (APTNPCCharacter* NearbyNPC = GetBestNearbyNPC())
        {
            PlayerCharacter->Server_TryInteract(NearbyNPC);
            UE_LOG(LogTemp, Log, TEXT("Controller: F input -> interact with nearby NPC %s"), *NearbyNPC->GetName());
            return;
        }

        if (APTInteractionActor* NearbyInteractionActor = GetBestNearbyInteractionActor())
        {
            PlayerCharacter->Server_TryInteract(NearbyInteractionActor);
            UE_LOG(LogTemp, Log, TEXT("Controller: F input -> interact with nearby actor %s"),
                *NearbyInteractionActor->GetName());
            return;
        }

        PlayerCharacter->TryInteract();
        UE_LOG(LogTemp, Log, TEXT("컨트롤러: F키 입력 감지 -> 캐릭터에게 상호작용 명령 전달"));
    }
}

void APTPlayerController::RegisterNearbyNPC(APTNPCCharacter* NPC)
{
    if (NPC == nullptr)
    {
        return;
    }

    NearbyNPCs.RemoveAll(
        [](const TObjectPtr<APTNPCCharacter>& NearbyNPC)
        {
            return !IsValid(NearbyNPC);
        });

    NearbyNPCs.AddUnique(NPC);
}

void APTPlayerController::UnregisterNearbyNPC(APTNPCCharacter* NPC)
{
    NearbyNPCs.RemoveAll(
        [NPC](const TObjectPtr<APTNPCCharacter>& NearbyNPC)
        {
            return !IsValid(NearbyNPC) || NearbyNPC == NPC;
        });
}

APTNPCCharacter* APTPlayerController::GetBestNearbyNPC() const
{
    const APawn* ControlledPawn = GetPawn();
    if (ControlledPawn == nullptr)
    {
        return nullptr;
    }

    APTNPCCharacter* BestNPC = nullptr;
    float BestDistanceSquared = TNumericLimits<float>::Max();
    const FVector PawnLocation = ControlledPawn->GetActorLocation();

    for (const TObjectPtr<APTNPCCharacter>& NearbyNPC : NearbyNPCs)
    {
        if (!IsValid(NearbyNPC))
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared(PawnLocation, NearbyNPC->GetActorLocation());
        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            BestNPC = NearbyNPC.Get();
        }
    }

    return BestNPC;
}

void APTPlayerController::RegisterNearbyInteractionActor(APTInteractionActor* InteractionActor)
{
    if (InteractionActor == nullptr)
    {
        return;
    }

    NearbyInteractionActors.RemoveAll(
        [](const TObjectPtr<APTInteractionActor>& NearbyActor)
        {
            return !IsValid(NearbyActor);
        });

    NearbyInteractionActors.AddUnique(InteractionActor);
}

void APTPlayerController::UnregisterNearbyInteractionActor(APTInteractionActor* InteractionActor)
{
    NearbyInteractionActors.RemoveAll(
        [InteractionActor](const TObjectPtr<APTInteractionActor>& NearbyActor)
        {
            return !IsValid(NearbyActor) || NearbyActor == InteractionActor;
        });
}

APTInteractionActor* APTPlayerController::GetBestNearbyInteractionActor() const
{
    const APawn* ControlledPawn = GetPawn();
    if (ControlledPawn == nullptr)
    {
        return nullptr;
    }

    APTInteractionActor* BestInteractionActor = nullptr;
    float BestDistanceSquared = TNumericLimits<float>::Max();
    const FVector PawnLocation = ControlledPawn->GetActorLocation();

    for (const TObjectPtr<APTInteractionActor>& NearbyActor : NearbyInteractionActors)
    {
        if (!IsValid(NearbyActor))
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared(PawnLocation, NearbyActor->GetActorLocation());
        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            BestInteractionActor = NearbyActor.Get();
        }
    }

    return BestInteractionActor;
}

/*void APTPlayerController::OnSkill1(const FInputActionValue& Value)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

      // 무기 장착 여부 검사
      if (!PC->EquipmentComponent || !PC->EquipmentComponent->IsWeaponEquipped())
      {
          return;
      }

    UE_LOG(LogTemp, Warning, TEXT("[Skill1] Slot0 = %s"), *PC->SkillComp->GetSkillAtSlot(0).ToString());
    PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(0));

    if (PC->SkillComp->GetCooldownRemaining(0) > 0.f) return;

    if (PC->SkillComp->bIsCooldown[0]) return;

    const FPTSkillRow* SkillData = PC->SkillComp->GetSkillData(PC->SkillComp->GetSkillAtSlot(0));
    if (SkillData && PC->CurrentMP < SkillData->MPCost) return;

    RotateTowardsMouse();

    PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(0));
}*/

void APTPlayerController::OnQuickSlot1()
{
    //포션 사용
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;
    if (UPTInventoryComponent* Inven = PC->GetInventoryComponent())
        Inven->UseFirstPotion();
}

void APTPlayerController::OnQuickSlot2()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;
    if (UPTInventoryComponent* Inven = PC->GetInventoryComponent())
        Inven->UseQuickSlot(1);
}

void APTPlayerController::OnDodge(const FInputActionValue& Value)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

    if (PC->SkillComp->GetCooldownRemaining(4) > 0.f) return;

    bMoveToDestination = false;
    StopMovement();

    RotateTowardsMouse();
    PC->SkillComp->TryDodge();
}

void APTPlayerController::Server_SetActorRotation_Implementation(FRotator NewRotation)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (PC)
    {
        PC->SetActorRotation(NewRotation);
    }
}

void APTPlayerController::Server_TryPickupItem_Implementation(APTDropItemActorBase* TargetItem)
{
    if (!IsValid(TargetItem) || TargetItem->GetWorld() != GetWorld() || !TargetItem->HasAuthority())
    {
        return;
    }

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;

    const FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    const FVector ItemLocation = TargetItem->GetActorLocation();
    const float Distance2D = FVector::Dist2D(PlayerLocation, ItemLocation);
    if (Distance2D > 250.0f || FMath::Abs(PlayerLocation.Z - ItemLocation.Z) > 300.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("아이템이 너무 멀리 있거나, 잘못된 위치에서 획득 요청이 들어왔습니다."));
        return;
    }

    if (!TargetItem->TryClaimPickup())
    {
        return;
    }

    UPTInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent();
    const FItemData ItemData = TargetItem->GetItemData();
    const int32 Quantity = TargetItem->GetDroppedQuantity();
    if (InventoryComponent != nullptr && !ItemData.Item_ID.IsNone() && Quantity > 0 &&
        InventoryComponent->TryAddItem(ItemData, Quantity))
    {
        TargetItem->Destroy();
        UE_LOG(LogTemp, Log, TEXT("아이템을 획득하였습니다."));
    }
    else
    {
        TargetItem->ReleasePickupClaim();
        UE_LOG(LogTemp, Warning, TEXT("[획득 실패] 인벤토리 공간 부족 또는 아이템 ID 누락"));
    }
}

// Server RPC 패킷 위변조 검증부
bool APTPlayerController::Server_TryPickupItem_Validate(APTDropItemActorBase* TargetItem)
{
    return true;
}

void APTPlayerController::RequestDropInventoryItem(
    int32 InventorySlotIndex,
    FName ExpectedItemID,
    int32 Count)
{
    if (!HasAuthority())
    {
        Server_DropInventoryItem(InventorySlotIndex, ExpectedItemID, Count);
        return;
    }

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    UPTInventoryComponent* InventoryComponent =
        PlayerCharacter != nullptr ? PlayerCharacter->GetInventoryComponent() : nullptr;
    if (PlayerCharacter == nullptr || InventoryComponent == nullptr)
    {
        return;
    }

    const TArray<FInventorySlot>& Slots = InventoryComponent->GetInventorySlots();
    if (!Slots.IsValidIndex(InventorySlotIndex))
    {
        return;
    }

    const FInventorySlot& Slot = Slots[InventorySlotIndex];
    if (Slot.IsEmpty() || Slot.ItemData.Item_ID != ExpectedItemID || Count <= 0 || Count > Slot.Quantity)
    {
        UE_LOG(LogTemp, Warning, TEXT("[InventoryDrop] Rejected stale or invalid slot request. Slot=%d ItemID=%s Count=%d"),
            InventorySlotIndex, *ExpectedItemID.ToString(), Count);
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    const FItemData DroppedItemData = Slot.ItemData;
    const FVector Forward = PlayerCharacter->GetActorForwardVector().GetSafeNormal2D();
    const FVector DropSpawnLocation = PlayerCharacter->GetActorLocation() + Forward * 150.0f + FVector(0.0f, 0.0f, 30.0f);
    const FTransform SpawnTransform(FRotator::ZeroRotator, DropSpawnLocation);

    TSubclassOf<APTDropItemActorBase> DropClass = ResolveInventoryDropActorClass(DroppedItemData);
    if (DropClass == nullptr)
    {
        DropClass = APTDropItemActorBase::StaticClass();
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Instigator = PlayerCharacter;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    APTDropItemActorBase* DroppedItem = World->SpawnActor<APTDropItemActorBase>(
        DropClass,
        SpawnTransform,
        SpawnParameters);
    if (DroppedItem == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[InventoryDrop] Failed to spawn drop actor. ItemID=%s"),
            *ExpectedItemID.ToString());
        return;
    }

    DroppedItem->InitializeDroppedItem(DroppedItemData, Count);
    if (!InventoryComponent->RemoveItemAtSlot(InventorySlotIndex, Count))
    {
        DroppedItem->Destroy();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[InventoryDrop] Dropped item. ItemID=%s Quantity=%d Slot=%d"),
        *DroppedItemData.Item_ID.ToString(), Count, InventorySlotIndex);
}

void APTPlayerController::Server_DropInventoryItem_Implementation(
    int32 InventorySlotIndex,
    FName ExpectedItemID,
    int32 Count)
{
    RequestDropInventoryItem(InventorySlotIndex, ExpectedItemID, Count);
}

bool APTPlayerController::Server_DropInventoryItem_Validate(
    int32 InventorySlotIndex,
    FName ExpectedItemID,
    int32 Count)
{
    return InventorySlotIndex >= 0 && InventorySlotIndex < 30 &&
        !ExpectedItemID.IsNone() && Count > 0 && Count <= 1000000;
}

void APTPlayerController::OnInventoryPressed()
{
    if (!IsLocalPlayerController()) return;

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP) return;

    UPTUIManagerSubsystem* UI = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UI || !InventoryClass) return;

    UI->ToggleInventory(InventoryClass);
}

void APTPlayerController::OnShopPressed()
{
    if (!IsLocalPlayerController()) return;

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP) return;

    UPTUIManagerSubsystem* UI = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UI || !ShopClass) return;

    UI->ToggleShop(ShopClass);
}

void APTPlayerController::OnQuestPressed()
{
    if (!IsLocalPlayerController()) return;

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP) return;

    UPTUIManagerSubsystem* UI = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UI || !QuestClass) return;

    UI->ToggleQuest(QuestClass);
}

void APTPlayerController::OnSettingsPressed()
{
    if (!IsLocalPlayerController()) return;

    if (SettingsInstance != nullptr &&
        (SettingsInstance->IsActivated() || SettingsInstance->IsInViewport()))
    {
        SettingsInstance->DeactivateWidget();
        SettingsInstance = nullptr;
        return;
    }

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP) return;

    UPTUIManagerSubsystem* UI = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UI || !SettingsClass) return;

    SettingsInstance = UI->PushWidget(SettingsClass, EPTUILayer::Modal);
}

void APTPlayerController::AddUIInputMapping()
{
    if (!IsLocalPlayerController()) return;
    if (bUIInputMappingAdded || !IMC_UI) return;

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer) return;

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!InputSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("EnhancedInputLocalPlayerSubsystem not found."));
        return;
    }

    InputSubsystem->AddMappingContext(IMC_UI, 100);
    bUIInputMappingAdded = true;

    UE_LOG(LogTemp, Warning, TEXT("UI Input Mapping Added: %s / Priority=100"), *IMC_UI->GetName());
}

void APTPlayerController::RemoveUIInputMapping()
{
    if (!bUIInputMappingAdded || !IMC_UI) return;

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer) return;

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!InputSubsystem) return;

    InputSubsystem->RemoveMappingContext(IMC_UI);
    bUIInputMappingAdded = false;
}

void APTPlayerController::Client_ShowMonsterHealth_Implementation(APTMonsterCharacter* Monster)
{
    OnMonsterTargeted.Broadcast(Monster);
}

// 유다이 UI
void APTPlayerController::Client_ShowDeathMenu_Implementation()
{
    if (!IsLocalPlayerController()) return;

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP || !DeathMenuClass) return;

    UPTUIManagerSubsystem* UI = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UI) return;

    UI->PushWidget(DeathMenuClass,EPTUILayer::Modal);
}

// 부활 요청
void APTPlayerController::Server_RequestRespawn_Implementation()
{
    UWorld* World = GetWorld();
    APTGameMode* GM = World != nullptr ? Cast<APTGameMode>(World->GetAuthGameMode()) : nullptr;
    if (!GM) return;

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());

    // 이미 캐릭터가 없거나 죽은 상태일 때만 처리
    if (!PlayerCharacter || PlayerCharacter->CurrentHP <= 0.f)
    {
        // [핵심] 캐릭터를 파괴하기 전에 PlayerState에서 세이브포인트 정보를 먼저 안전하게 읽어옵니다.
        FVector TargetRespawnLocation = FVector::ZeroVector;
        bool bHasSavedLocation = false;

        if (APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>())
        {
            if (PS->HasRespawnLocation())
            {
                TargetRespawnLocation = PS->GetSavedRespawnLocation();
                bHasSavedLocation = true;
            }
        }

        // 이제 기존 캐릭터를 안전하게 파괴합니다.
        if (PlayerCharacter)
        {
            PlayerCharacter->Destroy();
        }

        // 세이브포인트가 있다면 위치를 명시해서 게임모드에 리스폰을 요청하고, 
        // 없다면 일반 리스폰(Default 플레이어 스타트)을 요청합니다.
        if (bHasSavedLocation)
        {
            // 게임모드에 구현된 세이브포인트 전용 리스폰 함수를 호출합니다.
            // (태그 이동이나 특정 위치 스폰을 보장하는 오버로딩 함수)
            GM->RespawnPlayer(this, TargetRespawnLocation, true);
        }
        else
        {
            GM->RespawnPlayer(this);
        }
    }
}

// [디버그] 즉사 입력
bool APTPlayerController::Server_RequestRespawn_Validate()
{
    return true;
}

void APTPlayerController::OnDebugKillPressed()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

    Server_DebugKill();
}

// [디버그] 즉사
void APTPlayerController::Server_DebugKill_Implementation()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

    // 방어력 무시하고 확실히 죽임
    PC->ApplyDamage(PC->MaxHP + PC->BaseDef + 1.f, nullptr);
}
void APTPlayerController::Server_SetReady_Implementation(bool bReady)
{
    if (APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>())
    {
        PS->SetReady(bReady);
    }
}

// 로비 채팅 추가
void APTPlayerController::Server_SendChatMessage_Implementation(const FString& Message)
{
    const FString Trimmed = Message.TrimStartAndEnd();
    if (Trimmed.IsEmpty() || Trimmed.Len() > 200)
    {
        return;
    }

    if (APTGameState* PTGameState = GetWorld()->GetGameState<APTGameState>())
    {
        const FString SenderName = PlayerState ? PlayerState->GetPlayerName() : TEXT("Unknown");
        PTGameState->Server_AddChatMessage(SenderName, Trimmed);
    }
}

//skill 창
void APTPlayerController::OnSkillWindowPressed()
{
    if (!IsLocalPlayerController()) return;

    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 1. OnSkillWindowPressed 호출됨"));

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP)
    {
        UE_LOG(LogTemp, Error, TEXT("[SkillWindow] 2. GetLocalPlayer() 실패 - NULL"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 2. LocalPlayer 확인됨"));

    UPTUIManagerSubsystem* UIManager = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UIManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[SkillWindow] 3. UIManagerSubsystem 실패 - NULL"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 3. UIManagerSubsystem 확인됨"));

    UE_LOG(LogTemp, Warning, TEXT("[SkillWindow] 4. SkillWindowClass=%s"),
        SkillWindowClass ? *SkillWindowClass->GetName() : TEXT("NULL!!"));

    UIManager->ToggleSkillWindow(SkillWindowClass);
}


void APTPlayerController::OnCharacterSheetPressed()
{
    if (!IsLocalPlayerController()) return;

    UE_LOG(LogTemp, Warning, TEXT("[CharacterSheet] 1. OnCharacterSheetPressed 호출됨"));

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP)
    {
        UE_LOG(LogTemp, Error, TEXT("[CharacterSheet] 2. GetLocalPlayer() 실패 - NULL"));
        return;
    }

    UPTUIManagerSubsystem* UIManager = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UIManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[CharacterSheet] 3. UIManagerSubsystem 실패 - NULL"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[CharacterSheet] 4. CharacterSheetClass=%s"),
        CharacterSheetClass ? *CharacterSheetClass->GetName() : TEXT("NULL!!"));

    UIManager->ToggleCharacterSheet(CharacterSheetClass);
}

void APTPlayerController::OnCloseMenuPressed()
{
    if (!IsLocalPlayerController()) return;

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP) return;

    UPTUIManagerSubsystem* UI = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UI || !CloseWidgetClass) return;

    UI->ToggleCloseWidget(CloseWidgetClass);
}

void APTPlayerController::Server_RequestAssignSkillToSlot_Implementation(FName SkillID, int32 SlotIndex)
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    if (UPTPlayerSkillComponent* SkillComp = MyPawn->FindComponentByClass<UPTPlayerSkillComponent>())
    {
        SkillComp->AssignSkillToSlot(SkillID, SlotIndex);
        SkillComp->Client_NotifySkillSlotAssigned(SlotIndex, SkillID);
    }
}

void APTPlayerController::HandleSkillPressed(int32 SlotIndex)
{
    if (bGameplayInputBlockedByUI || IsMouseOverGameplayUI())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] UI에 막힘")); return;
    }

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC || !PC->SkillComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] PC/SkillComp 없음")); return;
    }

    const FName SkillID = PC->SkillComp->GetSkillAtSlot(SlotIndex);
    if (SkillID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] 슬롯 %d 비어있음"), SlotIndex); return;
    }

    if (PC->SkillComp->GetCooldownRemaining(SlotIndex) > 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] 쿨다운 중")); return;
    }

    const FPTSkillRow* Row = PC->SkillComp->GetSkillData(SkillID);
    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] Row 없음")); return;
    }

    if (PC->CurrentMP < Row->MPCost)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] MP 부족 %.0f/%.0f"), PC->CurrentMP, Row->MPCost); return;
    }

    if (Row->bIsChanneled)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] 채널 시작 요청 슬롯 %d, %s"), SlotIndex, *SkillID.ToString());
        PC->SkillComp->Server_StartChannelSkill(SlotIndex, SkillID);
        return;
    }

    // 인디케이터 없거나 즉시시전 -> 기존 동작 (커서 방향 바로 발사)
    if (Row->IndicatorShape == ESkillIndicatorShape::None || Row->bQuickCast)
    {
        FVector Ground;
        FVector Dir = PC->GetActorForwardVector();
        FVector Point = PC->GetActorLocation();
        if (GetGroundPointUnderCursor(Ground))
        {
            FVector To = Ground - PC->GetActorLocation(); To.Z = 0.f;
            if (!To.IsNearlyZero()) { Dir = To.GetSafeNormal(); Point = Ground; }
        }
        PC->SetActorRotation(Dir.Rotation());
        Server_SetActorRotation(Dir.Rotation());
        PC->Server_UseSkill(SkillID, Point, Dir, nullptr);
        return;
    }

    BeginSkillAim(SlotIndex);   // 조준 모드
}

void APTPlayerController::HandleSkillReleased(int32 SlotIndex)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
    {
        if (PC->SkillComp)
        {
            const FName SkillID = PC->SkillComp->GetSkillAtSlot(SlotIndex);
            if (const FPTSkillRow* Row = PC->SkillComp->GetSkillData(SkillID))
            {
                if (Row->bIsChanneled)
                {
                    PC->SkillComp->Server_EndChannelSkill();
                    return;
                }
            }
        }
    }

    if (bIsAiming && AimingSlotIndex == SlotIndex)   // 기존 조준 확정
        ConfirmSkillAim();
}

void APTPlayerController::BeginSkillAim(int32 SlotIndex)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

    if (!IndicatorActor && IndicatorActorClass)
    {
        FActorSpawnParameters P; P.Owner = PC;
        IndicatorActor = GetWorld()->SpawnActor<APTSkillIndicatorActor>(IndicatorActorClass, FTransform::Identity, P);
    }

    bIsAiming = true;
    AimingSlotIndex = SlotIndex;
    AimingSkillID = PC->SkillComp->GetSkillAtSlot(SlotIndex);
    CachedTarget = nullptr;
    UpdateSkillAim();
}

void APTPlayerController::UpdateSkillAim()
{
    if (!bIsAiming || !IndicatorActor) return;
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    const FPTSkillRow* Row = PC ? PC->SkillComp->GetSkillData(AimingSkillID) : nullptr;
    if (!PC || !Row) return;

    const FVector PlayerChar = PC->GetActorLocation();

    // 사거리 원 (캐릭터 중심, 고정)
    IndicatorActor->ShowRange(PlayerChar, Row->CastRange);

    FVector Ground;
    if (!GetGroundPointUnderCursor(Ground)) return;

    FVector To = Ground - PlayerChar; To.Z = 0.f;
    const float Dist = To.Size();
    const FVector Dir = (Dist > 1.f) ? To / Dist : PC->GetActorForwardVector();
    const float Clamped = FMath::Min(Dist, Row->CastRange);
    const FVector Point = PlayerChar + Dir * Clamped;

    UWorld* W = GetWorld();
    const FColor C = FColor::Cyan;

    if (Row->IndicatorShape == ESkillIndicatorShape::Circle)
    {
        IndicatorActor->ShowRange(PlayerChar, Row->CastRange);
    }
    else
    {
        IndicatorActor->HideRange();
    }

    switch (Row->IndicatorShape)
    {
    case ESkillIndicatorShape::Line:
        IndicatorActor->ShowLine(PlayerChar, Dir, Row->CastRange, Row->IndicatorWidth);
        break;

    case ESkillIndicatorShape::Circle:
        IndicatorActor->ShowCircle(Point, Row->SkillRadius);
        break;

    case ESkillIndicatorShape::Cone:
        IndicatorActor->ShowCone(PlayerChar, Dir, Row->CastRange, Row->IndicatorWidth);
        break;

    case ESkillIndicatorShape::SelfCircle:
        IndicatorActor->ShowSelfCircle(PlayerChar, Row->SkillRadius);
        break;

    default:
        break;
    }

    // 캐릭터가 조준 방향 바라보게 (로컬만; 서버는 확정 시)
    PC->SetActorRotation(Dir.Rotation());

    if (Row->TargetingMode == ESkillTargetingMode::Targeted)
        CachedTarget = FindTargetUnderCursor(Row->CastRange);
}

void APTPlayerController::ConfirmSkillAim()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    const FPTSkillRow* Row = PC ? PC->SkillComp->GetSkillData(AimingSkillID) : nullptr;
    if (!PC || !Row) { CancelSkillAim(); return; }

    const FVector PlayerChar = PC->GetActorLocation();
    FVector Ground; GetGroundPointUnderCursor(Ground);
    FVector To = Ground - PlayerChar; To.Z = 0.f;
    const FVector Dir = To.IsNearlyZero() ? PC->GetActorForwardVector() : To.GetSafeNormal();
    const float Clamped = FMath::Min(To.Size(), Row->CastRange);
    const FVector Point = PlayerChar + Dir * Clamped;

    AActor* Target = (Row->TargetingMode == ESkillTargetingMode::Targeted) ? CachedTarget : nullptr;

    Server_SetActorRotation(Dir.Rotation());
    PC->Server_UseSkill(AimingSkillID, Point, Dir, Target);
    CancelSkillAim();
}

void APTPlayerController::CancelSkillAim()
{
    bIsAiming = false;
    AimingSlotIndex = INDEX_NONE;
    AimingSkillID = NAME_None;
    CachedTarget = nullptr;
    if (IndicatorActor) IndicatorActor->HideIndicator();
}

bool APTPlayerController::GetGroundPointUnderCursor(FVector& OutPoint) const
{
    FHitResult Hit;
    if (const_cast<APTPlayerController*>(this)->GetHitResultUnderCursor(ECC_Visibility, false, Hit) && Hit.bBlockingHit)
    {
        OutPoint = Hit.Location;
        return true;
    }
    return false;
}

AActor* APTPlayerController::FindTargetUnderCursor(float MaxRange) const
{
    FHitResult Hit;
    if (!const_cast<APTPlayerController*>(this)->GetHitResultUnderCursor(ECC_Pawn, false, Hit)) return nullptr;

    APTBaseCharacter* Enemy = Cast<APTBaseCharacter>(Hit.GetActor());
    if (!Enemy || Cast<APTPlayerCharacter>(Enemy)) return nullptr;
    if (FVector::Dist2D(GetPawn()->GetActorLocation(), Enemy->GetActorLocation()) > MaxRange) return nullptr;
    return Enemy;
}
