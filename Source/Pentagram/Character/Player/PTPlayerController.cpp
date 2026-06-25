#include "Character/Player/PTPlayerController.h"
#include "CommonActivatableWidget.h"
#include "Character/Player/PTBasePlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "PTBasePlayerState.h"
#include "PTEquipmentComponent.h"
#include "PTInventoryComponent.h"
#include "PTPlayerCharacter.h"
#include "UI/Widget/LayOut/PTPrimaryLayout.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Item/PTDropItemActorBase.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/NPC/PTQuestNPCCharacter.h"
#include "Character/NPC/PTShopNPCCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/PTGameMode.h"
#include "Core/Subsystems/PTEconomySubsystem.h"
#include "Core/Subsystems/PTItemSubsystem.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/PTHUDWidget.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Widget/Inventory/PTInventoryWidget.h"
#include "UI/Widget/NPC/PTNPCDialogueWidget.h"
#include "UI/Widget/Shop/PTShopWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "TimerManager.h"

APTPlayerController::APTPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
}

void APTPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalPlayerController()) return;

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

                    // 현재 레벨에 맞는 UI
                    const FString MapName = UGameplayStatics::GetCurrentLevelName(this, true);
                    UIMgr->OpenUILevel(FName(*MapName));
                }
            }
        }
    }
}

void APTPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bMoveToDestination) return;

    ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
    if (!MyCharacter) return;

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
        if (IA_Dodge)     EnhancedInput->BindAction(IA_Dodge,     ETriggerEvent::Started,   this, &APTPlayerController::OnDodge);
        if (IA_Skill1)    EnhancedInput->BindAction(IA_Skill1,    ETriggerEvent::Started,   this, &APTPlayerController::OnSkill1);
        if (IA_Skill2)    EnhancedInput->BindAction(IA_Skill2,    ETriggerEvent::Started,   this, &APTPlayerController::OnSkill2);
        if (IA_Skill3)    EnhancedInput->BindAction(IA_Skill3,    ETriggerEvent::Started,   this, &APTPlayerController::OnSkill3);
        if (IA_Skill4)    EnhancedInput->BindAction(IA_Skill4,    ETriggerEvent::Started,   this, &APTPlayerController::OnSkill4);

        // [디버그] 즉사
        if (IA_DebugKill) EnhancedInput->BindAction(IA_DebugKill, ETriggerEvent::Started,   this, &APTPlayerController::OnDebugKillPressed);

        if (IA_Interact)
        {
            UE_LOG(LogTemp, Warning, TEXT("IA_Interact Binding (F Key)"));
            EnhancedInput->BindAction(IA_Interact, ETriggerEvent::Started, this, &APTPlayerController::OnInteractPressed);
        }
    }

    AddUIInputMapping();
}

void APTPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);
    UE_LOG(LogTemp, Warning, TEXT("AcknowledgePossession Called"));

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (IMC_Default)
        {
            Subsystem->AddMappingContext(IMC_Default, 0);
            UE_LOG(LogTemp, Warning, TEXT("IMC Added"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("IMC_Default is null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Subsystem is null"));
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
}

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

void APTPlayerController::OnRightClick(const FInputActionValue& Value)
{
    if (bGameplayInputBlockedByUI) return;

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;
    if (PC->bIsDodging) return;

    UAnimMontage* PlayingAttackMontage = nullptr;
    if (USkeletalMeshComponent* MeshComponent = PC->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
        {
            for (const TObjectPtr<UAnimMontage>& AttackMontage : PC->AttackMontages)
            {
                UAnimMontage* Montage = AttackMontage.Get();
                if (IsValid(Montage) && AnimInstance->Montage_IsPlaying(Montage))
                {
                    PlayingAttackMontage = Montage;
                    break;
                }
            }
        }
    }

    if (PC->bIsAttacking || IsValid(PlayingAttackMontage))
    {
        PC->StopAnimMontage(PlayingAttackMontage);
        PC->Server_StopAttack();
        PC->bIsAttacking = false;
        PC->bCanCombo    = false;
        PC->ComboIndex   = 0;
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

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;
    if (PlayerCharacter->bIsDodging) return;
    if (PlayerCharacter->bIsUsingSkill) return;

    /*  무기 장착 여부 검사
    if (!PlayerCharacter->EquipmentComponent || !PlayerCharacter->EquipmentComponent->IsWeaponEquipped())
    {
        return;
    }
    */

    bMoveToDestination = false;
    StopMovement();

    if (PlayerCharacter->bIsAttacking && !PlayerCharacter->bCanCombo) return;

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

        // 마우스 지정 대상이 오브젝트가 드롭 아이템 액터인지 판별
        APTDropItemActorBase* TargetItem = Cast<APTDropItemActorBase>(HitResult.GetActor());
        if (TargetItem)
        {
            // 캐릭터와 아이템 간의 평면(2D) 거리 확인
            float Distance2D = FVector::Dist2D(PlayerCharacter->GetActorLocation(), TargetItem->GetActorLocation());

            if (Distance2D <= 250.0f) // 범위 판정
            {
                // 로컬에서 판단을 내리지 않고, 서버 RPC 전송
                Server_TryPickupItem(TargetItem);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("아이템이 너무 멀리 있습니다."));
            }
            return; // 아이템 클릭 시 공격 차단
        }
    }


    if (PlayerCharacter->bIsAttacking)
    {
        if (PlayerCharacter->bCanCombo)
        {
            PlayerCharacter->bCanCombo = false;
            PlayAttackMontage();
        }
        return;
    }
    PlayAttackMontage();
}

void APTPlayerController::OnInteractPressed()
{
    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (PlayerCharacter)
    {
        PlayerCharacter->TryInteract();
        UE_LOG(LogTemp, Log, TEXT("컨트롤러: F키 입력 감지 -> 캐릭터에게 상호작용 명령 전달"));
    }
}

void APTPlayerController::OnSkill1(const FInputActionValue& Value)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

    /*  무기 장착 여부 검사
      if (!PlayerCharacter->EquipmentComponent || !PlayerCharacter->EquipmentComponent->IsWeaponEquipped())
      {
          return;
      }
      */

    if (PC->SkillComp->GetCooldownRemaining(0) > 0.f) return;

    if (PC->SkillComp->bIsCooldown[0]) return;

    const FPTSkillRow* SkillData = PC->SkillComp->GetSkillData(PC->SkillComp->GetSkillAtSlot(0));
    if (SkillData && PC->CurrentMP < SkillData->MPCost) return;

    RotateTowardsMouse();

    PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(0));
}

void APTPlayerController::OnSkill2(const FInputActionValue& Value)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

    /*  무기 장착 여부 검사
      if (!PlayerCharacter->EquipmentComponent || !PlayerCharacter->EquipmentComponent->IsWeaponEquipped())
      {
          return;
      }
      */

    if (PC->SkillComp->GetCooldownRemaining(0) > 0.f) return;

    if (PC->SkillComp->bIsCooldown[1]) return;

    const FPTSkillRow* SkillData = PC->SkillComp->GetSkillData(PC->SkillComp->GetSkillAtSlot(1));
    if (SkillData && PC->CurrentMP < SkillData->MPCost) return;

    RotateTowardsMouse();
    PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(1));
}

void APTPlayerController::OnSkill3(const FInputActionValue& Value)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

    /*  무기 장착 여부 검사
      if (!PlayerCharacter->EquipmentComponent || !PlayerCharacter->EquipmentComponent->IsWeaponEquipped())
      {
          return;
      }
      */

    if (PC->SkillComp->GetCooldownRemaining(0) > 0.f) return;

    if (PC->SkillComp->bIsCooldown[2]) return;

    const FPTSkillRow* SkillData = PC->SkillComp->GetSkillData(PC->SkillComp->GetSkillAtSlot(2));
    if (SkillData && PC->CurrentMP < SkillData->MPCost) return;

    RotateTowardsMouse();
    PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(2));
}

void APTPlayerController::OnSkill4(const FInputActionValue& Value)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn());
    if (!PC) return;

    /*  무기 장착 여부 검사
      if (!PlayerCharacter->EquipmentComponent || !PlayerCharacter->EquipmentComponent->IsWeaponEquipped())
      {
          return;
      }
      */

    if (PC->SkillComp->GetCooldownRemaining(0) > 0.f) return;

    if (PC->SkillComp->bIsCooldown[3]) return;

    const FPTSkillRow* SkillData = PC->SkillComp->GetSkillData(PC->SkillComp->GetSkillAtSlot(3));
    if (SkillData && PC->CurrentMP < SkillData->MPCost) return;

    RotateTowardsMouse();
    PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(3));
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

// 서버에서 아이템 획득 시도 처리
void APTPlayerController::Server_TryPickupItem_Implementation(APTDropItemActorBase* TargetItem)
{
    if (!TargetItem) return;

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;

    // 서버에서도 캐릭터와 아이템 간의 거리가 유효한지 검증
    float Distance2D = FVector::Dist2D(PlayerCharacter->GetActorLocation(), TargetItem->GetActorLocation());
    if (Distance2D > 250.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("아이템이 너무 멀리 있거나, 잘못된 위치에서 획득 요청이 들어왔습니다."));
        return;
    }

    // 서버 권한으로 인벤토리에 안전하게 아이템 집어넣기 시도
    if (PlayerCharacter->GetInventoryComponent() &&
        PlayerCharacter->GetInventoryComponent()->TryAddItem(TargetItem->GetItemData(), 1))
    {
        TargetItem->Destroy();
        UE_LOG(LogTemp, Log, TEXT("아이템을 획득하였습니다."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[획득 실패] 인벤토리 공간 부족 또는 아이템 ID 누락"));
    }
}

// Server RPC 패킷 위변조 검증부
bool APTPlayerController::Server_TryPickupItem_Validate(APTDropItemActorBase* TargetItem)
{
    return true;
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

    // 사망 검증
    APawn* DeadPawn = GetPawn();

    if (APTBaseCharacter* Dead = Cast<APTBaseCharacter>(DeadPawn))
    {
        if (Dead->CurrentHP > 0.f) return; // 생존 시 차단
    }

    if (APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>())
    {
        PS->CurrentHP = PS->MaxHP;
        PS->BroadcastAllStats(); // 클라이언트 UI 및 스탯 동기화 강제 브로드캐스팅
    }

    // 시체 정리
    if (DeadPawn)
    {
        UnPossess();
        DeadPawn->Destroy();
    }

    GM->RespawnPlayer(this);
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
