#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Character/Player/PTInventoryComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Player/PTPlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Item/PTDropItemActorBase.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPTInventoryFieldDropMultiplayerTest,
    "Pentagram.Inventory.FieldDrop.MultiplayerTransfer",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FPTInventoryFieldDropMultiplayerTest::RunTest(const FString& Parameters)
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

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.ObjectFlags |= RF_Transient;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    const FVector TestLocation(100000.0f, 100000.0f, 500.0f);
    APTPlayerController* DropperController =
        TestWorld->SpawnActor<APTPlayerController>(APTPlayerController::StaticClass(), SpawnParameters);
    APTPlayerCharacter* DropperCharacter = TestWorld->SpawnActor<APTPlayerCharacter>(
        APTPlayerCharacter::StaticClass(),
        FTransform(FRotator::ZeroRotator, TestLocation),
        SpawnParameters);
    APTPlayerController* PickerController =
        TestWorld->SpawnActor<APTPlayerController>(APTPlayerController::StaticClass(), SpawnParameters);
    APTPlayerCharacter* PickerCharacter = TestWorld->SpawnActor<APTPlayerCharacter>(
        APTPlayerCharacter::StaticClass(),
        FTransform(FRotator::ZeroRotator, TestLocation),
        SpawnParameters);

    const bool bActorsSpawned =
        TestNotNull(TEXT("Spawn dropper controller"), DropperController) &&
        TestNotNull(TEXT("Spawn dropper character"), DropperCharacter) &&
        TestNotNull(TEXT("Spawn picker controller"), PickerController) &&
        TestNotNull(TEXT("Spawn picker character"), PickerCharacter);
    if (!bActorsSpawned)
    {
        if (DropperController != nullptr) DropperController->Destroy();
        if (DropperCharacter != nullptr) DropperCharacter->Destroy();
        if (PickerController != nullptr) PickerController->Destroy();
        if (PickerCharacter != nullptr) PickerCharacter->Destroy();
        return false;
    }

    DropperController->Possess(DropperCharacter);
    PickerController->Possess(PickerCharacter);

    FItemData TestItem;
    TestItem.Item_ID = TEXT("TestMultiplayerFieldDropItem");
    TestItem.Item_Name = FText::FromString(TEXT("Multiplayer Field Drop Test Item"));
    TestItem.Item_Category = EItemCategory::Consumable;
    TestItem.Item_Type = EItemType::Potion;
    TestItem.Item_Grade = EItemGrade::Rare;
    TestItem.Item_Bonus_Options.Add(TEXT("STR+7"));

    UPTInventoryComponent* DropperInventory = DropperCharacter->GetInventoryComponent();
    UPTInventoryComponent* PickerInventory = PickerCharacter->GetInventoryComponent();
    if (!TestNotNull(TEXT("Find dropper inventory"), DropperInventory) ||
        !TestNotNull(TEXT("Find picker inventory"), PickerInventory))
    {
        DropperController->Destroy();
        DropperCharacter->Destroy();
        PickerController->Destroy();
        PickerCharacter->Destroy();
        return false;
    }

    TestTrue(TEXT("Add source item stack"), DropperInventory->TryAddItem(TestItem, 3));

    int32 SourceSlotIndex = INDEX_NONE;
    const TArray<FInventorySlot>& SourceSlots = DropperInventory->GetInventorySlots();
    for (int32 Index = 0; Index < SourceSlots.Num(); ++Index)
    {
        if (!SourceSlots[Index].IsEmpty() && SourceSlots[Index].ItemData.Item_ID == TestItem.Item_ID)
        {
            SourceSlotIndex = Index;
            break;
        }
    }

    const bool bFoundSourceSlot = SourceSlotIndex != INDEX_NONE;
    if (TestTrue(TEXT("Find source inventory slot"), bFoundSourceSlot))
    {
        DropperController->RequestDropInventoryItem(SourceSlotIndex, TestItem.Item_ID, 3);
    }

    APTDropItemActorBase* DroppedActor = nullptr;
    for (TActorIterator<APTDropItemActorBase> It(TestWorld); It; ++It)
    {
        if (It->GetItemData().Item_ID == TestItem.Item_ID)
        {
            DroppedActor = *It;
            break;
        }
    }

    TestEqual(TEXT("Source inventory item removed"), DropperInventory->GetItemCount(TestItem.Item_ID), 0);
    if (TestNotNull(TEXT("Spawn replicated field item"), DroppedActor))
    {
        TestTrue(TEXT("Field item replication enabled"), DroppedActor->GetIsReplicated());
        TestTrue(TEXT("Potion uses the existing potion drop Blueprint"),
            DroppedActor->GetClass()->GetName().Contains(TEXT("BP_DropItem_Potion")));
        TestEqual(TEXT("Field item quantity preserved"), DroppedActor->GetDroppedQuantity(), 3);
        TestEqual(TEXT("Field item random option preserved"),
            DroppedActor->GetItemData().Item_Bonus_Options[0], FString(TEXT("STR+7")));

        PickerCharacter->SetActorLocation(DroppedActor->GetActorLocation());
        PickerController->Server_TryPickupItem(DroppedActor);

        TestEqual(TEXT("Other player receives full stack"), PickerInventory->GetItemCount(TestItem.Item_ID), 3);
        TestTrue(TEXT("Picked field actor is destroyed"), DroppedActor->IsActorBeingDestroyed());
    }

    DropperController->Destroy();
    DropperCharacter->Destroy();
    PickerController->Destroy();
    PickerCharacter->Destroy();
    return true;
}

#endif
