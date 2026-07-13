#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTEquipmentComponent.h"
#include "Character/Player/PTInventoryComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Core/Subsystems/PTSaveGame.h"
#include "Core/Subsystems/PTSaveSubsystem.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widget/Inventory/PTEquipSlotWidget.h"
#include "UI/Widget/Inventory/PTInventoryWidget.h"
#include "UObject/UObjectIterator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPTSaveGameCharacterDataRoundTripTest,
    "Pentagram.Save.CharacterDataRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPTSaveGameCharacterDataRoundTripTest::RunTest(const FString& Parameters)
{
    UPTSaveGame* SourceSaveGame = Cast<UPTSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UPTSaveGame::StaticClass()));
    if (!TestNotNull(TEXT("Create save game object"), SourceSaveGame))
    {
        return false;
    }

    FPTPlayerSaveData& SourceData = SourceSaveGame->SaveData;
    SourceData.SaveVersion = 2;
    SourceData.Gold = 3587;
    SourceData.Level = 4;
    SourceData.Exp = 50;

    FPTQuestProgress InProgressQuest;
    InProgressQuest.QuestID = TEXT("TestKillQuest");
    InProgressQuest.State = EPTQuestProgressState::InProgress;
    FPTQuestConditionProgress InProgressCondition;
    InProgressCondition.ConditionType = EPTQuestConditionType::KillMonster;
    InProgressCondition.TargetID = TEXT("TestMonster");
    InProgressCondition.CurrentCount = 4;
    InProgressCondition.RequiredCount = 5;
    InProgressQuest.Conditions.Add(InProgressCondition);
    SourceData.AcceptedQuests.Add(InProgressQuest);

    FPTQuestProgress RewardedQuest = InProgressQuest;
    RewardedQuest.QuestID = TEXT("TestRewardedQuest");
    RewardedQuest.State = EPTQuestProgressState::Rewarded;
    RewardedQuest.Conditions[0].CurrentCount = 5;
    SourceData.AcceptedQuests.Add(RewardedQuest);

    SourceData.bHasInventoryData = true;
    SourceData.InventorySlots.Init(FInventorySlot(), 30);
    FInventorySlot& InventorySlot = SourceData.InventorySlots[4];
    InventorySlot.ItemData.Item_ID = TEXT("TestPotion");
    InventorySlot.ItemData.Item_Category = EItemCategory::Consumable;
    InventorySlot.ItemData.Item_Type = EItemType::Potion;
    InventorySlot.Quantity = 7;

    SourceData.bHasEquipmentData = true;
    FEquipmentSlot WeaponSlot(EEquipSlotType::Weapon);
    WeaponSlot.bIsEquipped = true;
    WeaponSlot.MountedItem.Item_ID = TEXT("TestRareSword");
    WeaponSlot.MountedItem.Item_Category = EItemCategory::Equipment;
    WeaponSlot.MountedItem.Item_Type = EItemType::Weapon;
    WeaponSlot.MountedItem.Item_Grade = EItemGrade::Rare;
    WeaponSlot.MountedItem.Item_Base_Stat = 12;
    WeaponSlot.MountedItem.Item_Bonus_Options.Add(TEXT("STR+5"));
    SourceData.EquipmentSlots.Add(WeaponSlot);

    SourceData.bHasSkillData = true;
    SourceData.LearnedSkills = { TEXT("TestFireSkill"), TEXT("TestIceSkill") };
    SourceData.SkillSlots.Init(NAME_None, 5);
    SourceData.SkillSlots[1] = TEXT("TestFireSkill");

    TArray<uint8> SaveBytes;
    if (!TestTrue(TEXT("Serialize save game to memory"), UGameplayStatics::SaveGameToMemory(SourceSaveGame, SaveBytes)))
    {
        return false;
    }

    UPTSaveGame* LoadedSaveGame = Cast<UPTSaveGame>(UGameplayStatics::LoadGameFromMemory(SaveBytes));
    if (!TestNotNull(TEXT("Load save game from memory"), LoadedSaveGame))
    {
        return false;
    }

    const FPTPlayerSaveData& LoadedData = LoadedSaveGame->SaveData;
    TestEqual(TEXT("Save version"), LoadedData.SaveVersion, 2);
    TestEqual(TEXT("Gold"), LoadedData.Gold, 3587);
    TestEqual(TEXT("Level"), LoadedData.Level, 4);
    TestEqual(TEXT("Exp"), LoadedData.Exp, 50);
    TestEqual(TEXT("Quest count"), LoadedData.AcceptedQuests.Num(), 2);
    TestEqual(TEXT("In-progress quest ID"), LoadedData.AcceptedQuests[0].QuestID, FName(TEXT("TestKillQuest")));
    TestEqual(TEXT("Quest current count"), LoadedData.AcceptedQuests[0].Conditions[0].CurrentCount, 4);
    TestEqual(TEXT("Quest required count"), LoadedData.AcceptedQuests[0].Conditions[0].RequiredCount, 5);
    TestEqual(TEXT("Rewarded quest state"), LoadedData.AcceptedQuests[1].State, EPTQuestProgressState::Rewarded);

    TestTrue(TEXT("Inventory data flag"), LoadedData.bHasInventoryData);
    TestEqual(TEXT("Inventory slot count"), LoadedData.InventorySlots.Num(), 30);
    TestEqual(TEXT("Inventory item ID"), LoadedData.InventorySlots[4].ItemData.Item_ID, FName(TEXT("TestPotion")));
    TestEqual(TEXT("Inventory quantity"), LoadedData.InventorySlots[4].Quantity, 7);

    TestTrue(TEXT("Equipment data flag"), LoadedData.bHasEquipmentData);
    TestEqual(TEXT("Equipment slot count"), LoadedData.EquipmentSlots.Num(), 1);
    TestTrue(TEXT("Weapon equipped"), LoadedData.EquipmentSlots[0].bIsEquipped);
    TestEqual(TEXT("Weapon item ID"), LoadedData.EquipmentSlots[0].MountedItem.Item_ID, FName(TEXT("TestRareSword")));
    TestEqual(TEXT("Weapon base stat"), LoadedData.EquipmentSlots[0].MountedItem.Item_Base_Stat, 12);
    TestEqual(TEXT("Weapon random option"), LoadedData.EquipmentSlots[0].MountedItem.Item_Bonus_Options[0], FString(TEXT("STR+5")));

    TestTrue(TEXT("Skill data flag"), LoadedData.bHasSkillData);
    TestEqual(TEXT("Learned skill count"), LoadedData.LearnedSkills.Num(), 2);
    TestEqual(TEXT("First learned skill"), LoadedData.LearnedSkills[0], FName(TEXT("TestFireSkill")));
    TestEqual(TEXT("Skill slot count"), LoadedData.SkillSlots.Num(), 5);
    TestEqual(TEXT("Assigned skill"), LoadedData.SkillSlots[1], FName(TEXT("TestFireSkill")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPTInventoryMutationDiskSaveTest,
    "Pentagram.Save.InventoryMutationDiskSave",
    EAutomationTestFlags::ClientContext |
        EAutomationTestFlags::EngineFilter)

bool FPTInventoryMutationDiskSaveTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = nullptr;
    if (GEngine != nullptr)
    {
        for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
        {
            UWorld* World = WorldContext.World();
            if (World != nullptr && World->IsGameWorld())
            {
                TestWorld = World;
                break;
            }
        }
    }

    if (!TestNotNull(TEXT("Find game world"), TestWorld))
    {
        return false;
    }

    UGameInstance* GameInstance = TestWorld->GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    if (!TestNotNull(TEXT("Find save subsystem"), SaveSubsystem))
    {
        return false;
    }

    constexpr int32 TestPlayerID = 987654321;
    const FString TestSlotName = FString::Printf(TEXT("PTPlayerSave_PlayerId_%d"), TestPlayerID);
    UGameplayStatics::DeleteGameInSlot(TestSlotName, 0);

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.ObjectFlags |= RF_Transient;
    APlayerController* Controller =
        TestWorld->SpawnActor<APlayerController>(APlayerController::StaticClass(), SpawnParameters);
    APTBasePlayerState* PTPlayerState =
        TestWorld->SpawnActor<APTBasePlayerState>(APTBasePlayerState::StaticClass(), SpawnParameters);
    APTPlayerCharacter* PlayerCharacter =
        TestWorld->SpawnActor<APTPlayerCharacter>(APTPlayerCharacter::StaticClass(), SpawnParameters);

    const bool bActorsSpawned =
        TestNotNull(TEXT("Spawn controller"), Controller) &&
        TestNotNull(TEXT("Spawn PlayerState"), PTPlayerState) &&
        TestNotNull(TEXT("Spawn player character"), PlayerCharacter);
    if (!bActorsSpawned)
    {
        if (Controller != nullptr)
        {
            Controller->Destroy();
        }
        if (PTPlayerState != nullptr)
        {
            PTPlayerState->Destroy();
        }
        if (PlayerCharacter != nullptr)
        {
            PlayerCharacter->Destroy();
        }
        return false;
    }

    Controller->Possess(PlayerCharacter);
    PTPlayerState->SetPlayerId(TestPlayerID);
    PTPlayerState->SetOwner(Controller);
    PlayerCharacter->SetPlayerState(PTPlayerState);

    FItemData TestItem;
    TestItem.Item_ID = TEXT("TestInventoryMutationItem");
    TestItem.Item_Name = FText::FromString(TEXT("Inventory Save Test Item"));
    TestItem.Item_Category = EItemCategory::Consumable;
    TestItem.Item_Type = EItemType::Potion;

    UPTInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent();
    TestNotNull(TEXT("Find inventory component"), InventoryComponent);
    if (InventoryComponent != nullptr)
    {
        TestTrue(TEXT("Add item and trigger save"), InventoryComponent->TryAddItem(TestItem, 3));
        TestTrue(TEXT("Save slot created"), UGameplayStatics::DoesSaveGameExist(TestSlotName, 0));

        InventoryComponent->RestoreInventorySlots(TArray<FInventorySlot>());
        TestEqual(TEXT("Inventory cleared before load"), InventoryComponent->GetItemCount(TestItem.Item_ID), 0);

        TestTrue(TEXT("Load player save"), SaveSubsystem->LoadPlayer(PTPlayerState));
        TestEqual(TEXT("Saved item restored"), InventoryComponent->GetItemCount(TestItem.Item_ID), 3);
    }

    PlayerCharacter->Destroy();
    Controller->Destroy();
    PTPlayerState->Destroy();
    UGameplayStatics::DeleteGameInSlot(TestSlotName, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPTInventoryRestoreBeforeBeginPlayTest,
    "Pentagram.Save.InventoryRestoreBeforeBeginPlay",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FPTInventoryRestoreBeforeBeginPlayTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = nullptr;
    if (GEngine != nullptr)
    {
        for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
        {
            UWorld* World = WorldContext.World();
            if (World != nullptr && World->IsGameWorld())
            {
                TestWorld = World;
                break;
            }
        }
    }

    if (!TestNotNull(TEXT("Find game world"), TestWorld))
    {
        return false;
    }

    APTPlayerCharacter* PlayerCharacter = TestWorld->SpawnActorDeferred<APTPlayerCharacter>(
        APTPlayerCharacter::StaticClass(),
        FTransform::Identity,
        nullptr,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!TestNotNull(TEXT("Spawn deferred player character"), PlayerCharacter))
    {
        return false;
    }

    TArray<FInventorySlot> SavedSlots;
    SavedSlots.Init(FInventorySlot(), 30);
    SavedSlots[6].ItemData.Item_ID = TEXT("TestPreBeginPlayItem");
    SavedSlots[6].ItemData.Item_Category = EItemCategory::Consumable;
    SavedSlots[6].ItemData.Item_Type = EItemType::Potion;
    SavedSlots[6].Quantity = 4;

    UPTInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent();
    TestNotNull(TEXT("Find inventory component"), InventoryComponent);
    if (InventoryComponent != nullptr)
    {
        TestTrue(TEXT("Restore inventory before BeginPlay"), InventoryComponent->RestoreInventorySlots(SavedSlots));
    }

    UGameplayStatics::FinishSpawningActor(PlayerCharacter, FTransform::Identity);

    if (InventoryComponent != nullptr)
    {
        TestEqual(
            TEXT("BeginPlay preserves restored inventory"),
            InventoryComponent->GetItemCount(TEXT("TestPreBeginPlayItem")),
            4);
    }

    PlayerCharacter->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPTQuestProgressDiskSaveTest,
    "Pentagram.Save.QuestProgressDiskSave",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FPTQuestProgressDiskSaveTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = nullptr;
    if (GEngine != nullptr)
    {
        for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
        {
            UWorld* World = WorldContext.World();
            if (World != nullptr && World->IsGameWorld())
            {
                TestWorld = World;
                break;
            }
        }
    }

    if (!TestNotNull(TEXT("Find game world"), TestWorld))
    {
        return false;
    }

    UGameInstance* GameInstance = TestWorld->GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    UPTQuestSubsystem* QuestSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTQuestSubsystem>() : nullptr;
    if (!TestNotNull(TEXT("Find save subsystem"), SaveSubsystem) ||
        !TestNotNull(TEXT("Find quest subsystem"), QuestSubsystem))
    {
        return false;
    }

    constexpr int32 TestPlayerID = 987654322;
    const FString TestSlotName = FString::Printf(TEXT("PTPlayerSave_PlayerId_%d"), TestPlayerID);
    UGameplayStatics::DeleteGameInSlot(TestSlotName, 0);

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.ObjectFlags |= RF_Transient;
    APlayerController* Controller =
        TestWorld->SpawnActor<APlayerController>(APlayerController::StaticClass(), SpawnParameters);
    APTBasePlayerState* PTPlayerState =
        TestWorld->SpawnActor<APTBasePlayerState>(APTBasePlayerState::StaticClass(), SpawnParameters);
    APTPlayerCharacter* PlayerCharacter =
        TestWorld->SpawnActor<APTPlayerCharacter>(APTPlayerCharacter::StaticClass(), SpawnParameters);

    const bool bActorsSpawned =
        TestNotNull(TEXT("Spawn controller"), Controller) &&
        TestNotNull(TEXT("Spawn PlayerState"), PTPlayerState) &&
        TestNotNull(TEXT("Spawn player character"), PlayerCharacter);
    if (!bActorsSpawned)
    {
        if (Controller != nullptr)
        {
            Controller->Destroy();
        }
        if (PTPlayerState != nullptr)
        {
            PTPlayerState->Destroy();
        }
        if (PlayerCharacter != nullptr)
        {
            PlayerCharacter->Destroy();
        }
        return false;
    }

    Controller->Possess(PlayerCharacter);
    PTPlayerState->SetPlayerId(TestPlayerID);
    PTPlayerState->SetOwner(Controller);
    PlayerCharacter->SetPlayerState(PTPlayerState);

    FPTQuestProgress QuestProgress;
    QuestProgress.QuestID = TEXT("TestPersistentKillQuest");
    QuestProgress.State = EPTQuestProgressState::InProgress;
    FPTQuestConditionProgress ConditionProgress;
    ConditionProgress.ConditionType = EPTQuestConditionType::KillMonster;
    ConditionProgress.TargetID = TEXT("TestPersistentMonster");
    ConditionProgress.RequiredCount = 5;
    QuestProgress.Conditions.Add(ConditionProgress);
    PTPlayerState->AcceptedQuests.Add(QuestProgress);

    TestTrue(
        TEXT("Update quest progress to four"),
        QuestSubsystem->UpdateQuestProgress(
            PTPlayerState,
            EPTQuestConditionType::KillMonster,
            TEXT("TestPersistentMonster"),
            4));
    TestTrue(TEXT("Quest progress save slot created"), UGameplayStatics::DoesSaveGameExist(TestSlotName, 0));

    PTPlayerState->AcceptedQuests.Empty();
    TestTrue(TEXT("Load four-kill quest progress"), SaveSubsystem->LoadPlayer(PTPlayerState));

    const FPTQuestProgress* LoadedQuestProgress =
        QuestSubsystem->GetQuestProgress(PTPlayerState, TEXT("TestPersistentKillQuest"));
    TestNotNull(TEXT("Find loaded quest progress"), LoadedQuestProgress);
    if (LoadedQuestProgress != nullptr)
    {
        TestEqual(TEXT("Loaded kill count"), LoadedQuestProgress->Conditions[0].CurrentCount, 4);
        TestEqual(TEXT("Loaded in-progress state"), LoadedQuestProgress->State, EPTQuestProgressState::InProgress);
    }

    TestTrue(
        TEXT("Complete quest and save state"),
        QuestSubsystem->CompleteQuest(PTPlayerState, TEXT("TestPersistentKillQuest")));
    PTPlayerState->AcceptedQuests.Empty();
    TestTrue(TEXT("Reload completed quest"), SaveSubsystem->LoadPlayer(PTPlayerState));

    LoadedQuestProgress = QuestSubsystem->GetQuestProgress(PTPlayerState, TEXT("TestPersistentKillQuest"));
    TestNotNull(TEXT("Find reloaded completed quest"), LoadedQuestProgress);
    if (LoadedQuestProgress != nullptr)
    {
        TestEqual(TEXT("Reloaded completed state"), LoadedQuestProgress->State, EPTQuestProgressState::Completed);
    }

    PlayerCharacter->Destroy();
    Controller->Destroy();
    PTPlayerState->Destroy();
    UGameplayStatics::DeleteGameInSlot(TestSlotName, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPTEquipmentWidgetRestoreTest,
    "Pentagram.Save.EquipmentWidgetRestore",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FPTEquipmentWidgetRestoreTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = nullptr;
    if (GEngine != nullptr)
    {
        for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
        {
            UWorld* World = WorldContext.World();
            if (World != nullptr && World->IsGameWorld())
            {
                TestWorld = World;
                break;
            }
        }
    }

    if (!TestNotNull(TEXT("Find game world"), TestWorld))
    {
        return false;
    }

    APlayerController* PlayerController = TestWorld->GetFirstPlayerController();
    APTPlayerCharacter* PlayerCharacter =
        PlayerController != nullptr ? Cast<APTPlayerCharacter>(PlayerController->GetPawn()) : nullptr;
    UPTEquipmentComponent* EquipmentComponent =
        PlayerCharacter != nullptr ? PlayerCharacter->GetEquipmentComponent() : nullptr;
    if (!TestNotNull(TEXT("Find local player controller"), PlayerController) ||
        !TestNotNull(TEXT("Find local player character"), PlayerCharacter) ||
        !TestNotNull(TEXT("Find equipment component"), EquipmentComponent))
    {
        return false;
    }

    const TArray<FEquipmentSlot> OriginalEquipmentSlots = EquipmentComponent->GetEquipmentSlots();

    FEquipmentSlot TestWeaponSlot(EEquipSlotType::Weapon);
    TestWeaponSlot.bIsEquipped = true;
    TestWeaponSlot.MountedItem.Item_ID = TEXT("TestEquipmentWidgetWeapon");
    TestWeaponSlot.MountedItem.Item_Name = FText::FromString(TEXT("Equipment Widget Test Weapon"));
    TestWeaponSlot.MountedItem.Item_Category = EItemCategory::Equipment;
    TestWeaponSlot.MountedItem.Item_Type = EItemType::Weapon;
    TestWeaponSlot.MountedItem.Item_Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(
        TEXT("/Game/Pentagram/UI/Image/Ico/T_UI_Sword.T_UI_Sword")));
    TestTrue(
        TEXT("Restore equipped weapon into component"),
        EquipmentComponent->RestoreEquipmentSlots({ TestWeaponSlot }));

    UClass* InventoryWidgetClass = LoadClass<UPTInventoryWidget>(
        nullptr,
        TEXT("/Game/Pentagram/UI/Widget/Inventory/WBP_PTInventoryWidget.WBP_PTInventoryWidget_C"));
    UPTInventoryWidget* InventoryWidget = InventoryWidgetClass != nullptr
        ? CreateWidget<UPTInventoryWidget>(PlayerController, InventoryWidgetClass)
        : nullptr;
    if (!TestNotNull(TEXT("Load inventory widget class"), InventoryWidgetClass) ||
        !TestNotNull(TEXT("Create inventory widget"), InventoryWidget))
    {
        EquipmentComponent->RestoreEquipmentSlots(OriginalEquipmentSlots);
        return false;
    }

    InventoryWidget->AddToViewport();
    InventoryWidget->ActivateWidget();

    UPTEquipSlotWidget* WeaponSlotWidget = nullptr;
    for (TObjectIterator<UPTEquipSlotWidget> It; It; ++It)
    {
        UPTEquipSlotWidget* EquipSlotWidget = *It;
        if (EquipSlotWidget != nullptr &&
            EquipSlotWidget->GetTypedOuter<UPTInventoryWidget>() == InventoryWidget &&
            EquipSlotWidget->GetAllowedType() == EItemType::Weapon)
        {
            WeaponSlotWidget = EquipSlotWidget;
            break;
        }
    }

    TestNotNull(TEXT("Find weapon slot widget"), WeaponSlotWidget);
    if (WeaponSlotWidget != nullptr)
    {
        TestFalse(TEXT("Restored weapon slot is not empty"), WeaponSlotWidget->IsEmpty());
        TestEqual(
            TEXT("Restored weapon appears in equipment widget"),
            WeaponSlotWidget->GetSlotData().ItemData.Item_ID,
            FName(TEXT("TestEquipmentWidgetWeapon")));
    }

    InventoryWidget->DeactivateWidget();
    InventoryWidget->RemoveFromParent();
    EquipmentComponent->RestoreEquipmentSlots(OriginalEquipmentSlots);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPTPlayerStateSeamlessTravelCopyTest,
    "Pentagram.Save.PlayerStateSeamlessTravelCopy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPTPlayerStateSeamlessTravelCopyTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = nullptr;
    if (GEngine != nullptr)
    {
        for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
        {
            UWorld* World = WorldContext.World();
            if (World != nullptr && (World->IsGameWorld() || World->WorldType == EWorldType::Editor))
            {
                TestWorld = World;
                break;
            }
        }
    }

    if (!TestNotNull(TEXT("Find test world"), TestWorld))
    {
        return false;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.ObjectFlags |= RF_Transient;
    APTBasePlayerState* SourcePlayerState =
        TestWorld->SpawnActor<APTBasePlayerState>(APTBasePlayerState::StaticClass(), SpawnParameters);
    APTBasePlayerState* DestinationPlayerState =
        TestWorld->SpawnActor<APTBasePlayerState>(APTBasePlayerState::StaticClass(), SpawnParameters);
    if (!TestNotNull(TEXT("Spawn source PlayerState"), SourcePlayerState) ||
        !TestNotNull(TEXT("Spawn destination PlayerState"), DestinationPlayerState))
    {
        if (SourcePlayerState != nullptr)
        {
            SourcePlayerState->Destroy();
        }
        if (DestinationPlayerState != nullptr)
        {
            DestinationPlayerState->Destroy();
        }
        return false;
    }

    SourcePlayerState->CurrentGold = 3587;
    SourcePlayerState->PlayerLevel = 5;
    SourcePlayerState->CurrentExp = 42;
    SourcePlayerState->RequiredExp = 500;
    SourcePlayerState->CurrentHP = 75.f;
    SourcePlayerState->MaxHP = 120.f;
    SourcePlayerState->CurrentMP = 30.f;
    SourcePlayerState->MaxMP = 80.f;
    SourcePlayerState->BaseAtk = 17.f;
    SourcePlayerState->BaseDef = 9.f;
    SourcePlayerState->CriticalChance = 0.25f;
    SourcePlayerState->CriticalATK = 2.f;
    SourcePlayerState->MoveSpeed = 650.f;

    FPTQuestProgress QuestProgress;
    QuestProgress.QuestID = TEXT("TestSeamlessQuest");
    SourcePlayerState->AcceptedQuests.Add(QuestProgress);
    const FVector RespawnLocation(100.f, 200.f, 300.f);
    SourcePlayerState->SetSavedRespawnLocation(RespawnLocation);

    SourcePlayerState->DispatchCopyProperties(DestinationPlayerState);

    TestEqual(TEXT("Copied gold"), DestinationPlayerState->CurrentGold, 3587);
    TestEqual(TEXT("Copied level"), DestinationPlayerState->PlayerLevel, 5);
    TestEqual(TEXT("Copied exp"), DestinationPlayerState->CurrentExp, 42);
    TestEqual(TEXT("Copied required exp"), DestinationPlayerState->RequiredExp, 500);
    TestEqual(TEXT("Copied HP"), DestinationPlayerState->CurrentHP, 75.f);
    TestEqual(TEXT("Copied max HP"), DestinationPlayerState->MaxHP, 120.f);
    TestEqual(TEXT("Copied MP"), DestinationPlayerState->CurrentMP, 30.f);
    TestEqual(TEXT("Copied max MP"), DestinationPlayerState->MaxMP, 80.f);
    TestEqual(TEXT("Copied attack"), DestinationPlayerState->BaseAtk, 17.f);
    TestEqual(TEXT("Copied defense"), DestinationPlayerState->BaseDef, 9.f);
    TestEqual(TEXT("Copied critical chance"), DestinationPlayerState->CriticalChance, 0.25f);
    TestEqual(TEXT("Copied critical damage"), DestinationPlayerState->CriticalATK, 2.f);
    TestEqual(TEXT("Copied move speed"), DestinationPlayerState->MoveSpeed, 650.f);
    TestEqual(TEXT("Copied quest count"), DestinationPlayerState->AcceptedQuests.Num(), 1);
    TestEqual(
        TEXT("Copied quest ID"),
        DestinationPlayerState->AcceptedQuests[0].QuestID,
        FName(TEXT("TestSeamlessQuest")));
    TestTrue(TEXT("Copied respawn flag"), DestinationPlayerState->HasRespawnLocation());
    TestTrue(
        TEXT("Copied respawn location"),
        DestinationPlayerState->GetSavedRespawnLocation().Equals(RespawnLocation));

    SourcePlayerState->Destroy();
    DestinationPlayerState->Destroy();
    return true;
}

#endif
