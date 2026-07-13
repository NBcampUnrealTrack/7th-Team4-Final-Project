#include "PTQuestSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTInventoryComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Controller.h"
#include "PTEconomySubsystem.h"
#include "PTItemSubsystem.h"
#include "PTPlayerLevelSubsystem.h"
#include "PTSaveSubsystem.h"

void UPTQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    RebuildQuestDataMap();
}

void UPTQuestSubsystem::SetQuestDataTable(UDataTable* InQuestDataTable)
{
    QuestDataTable = InQuestDataTable;
    RebuildQuestDataMap();
}

void UPTQuestSubsystem::RebuildQuestDataMap()
{
    QuestDataMap.Empty();

    if (QuestDataTable == nullptr)
    {
        return;
    }

    TArray<FPTQuestDataRow*> QuestRows;
    QuestDataTable->GetAllRows<FPTQuestDataRow>(TEXT("Quest Data Map"), QuestRows);

    for (const FPTQuestDataRow* QuestRow : QuestRows)
    {
        if (QuestRow == nullptr || QuestRow->QuestID.IsNone())
        {
            continue;
        }

        QuestDataMap.Add(QuestRow->QuestID, *QuestRow);
    }
}

const FPTQuestDataRow* UPTQuestSubsystem::GetQuestData(FName QuestID) const
{
    if (QuestID.IsNone())
    {
        return nullptr;
    }

    return QuestDataMap.Find(QuestID);
}

bool UPTQuestSubsystem::HasQuestData(FName QuestID) const
{
    return GetQuestData(QuestID) != nullptr;
}

bool UPTQuestSubsystem::AcceptQuest(APTBasePlayerState* PlayerState, FName QuestID)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority() || QuestID.IsNone() ||
        GetQuestProgress(PlayerState, QuestID) != nullptr)
    {
        return false;
    }

    const FPTQuestDataRow* QuestData = GetQuestData(QuestID);
    if (QuestData == nullptr || !AreQuestRequirementsMet(PlayerState, QuestID))
    {
        return false;
    }

    PlayerState->AcceptedQuests.Add(MakeQuestProgress(*QuestData));
    PlayerState->ForceNetUpdate();
    OnQuestAccepted.Broadcast(QuestID);
    OnQuestListChanged.Broadcast();
    SaveQuestPlayerState(PlayerState, TEXT("Accepted"));
    return true;
}

bool UPTQuestSubsystem::HasAcceptedQuest(const APTBasePlayerState* PlayerState, FName QuestID) const
{
    return GetQuestProgress(PlayerState, QuestID) != nullptr;
}

const FPTQuestProgress* UPTQuestSubsystem::GetQuestProgress(const APTBasePlayerState* PlayerState, FName QuestID) const
{
    if (PlayerState == nullptr || QuestID.IsNone())
    {
        return nullptr;
    }

    return PlayerState->AcceptedQuests.FindByPredicate([QuestID](const FPTQuestProgress& QuestProgress)
        {
            return QuestProgress.QuestID == QuestID;
        });
}

bool UPTQuestSubsystem::UpdateQuestProgress(
    APTBasePlayerState* PlayerState,
    EPTQuestConditionType ConditionType,
    FName TargetID,
    int32 Amount)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority() ||
        ConditionType == EPTQuestConditionType::None || Amount <= 0)
    {
        return false;
    }

    bool bUpdatedAnyQuest = false;

    for (FPTQuestProgress& QuestProgress : PlayerState->AcceptedQuests)
    {
        if (QuestProgress.State != EPTQuestProgressState::InProgress)
        {
            continue;
        }

        bool bUpdatedQuest = false;
        for (FPTQuestConditionProgress& ConditionProgress : QuestProgress.Conditions)
        {
            const bool bMatchesType = ConditionProgress.ConditionType == ConditionType;
            const bool bMatchesTarget = ConditionProgress.TargetID.IsNone() || ConditionProgress.TargetID == TargetID;
            const bool bAlreadyComplete = ConditionProgress.CurrentCount >= ConditionProgress.RequiredCount;
            if (!bMatchesType || !bMatchesTarget || bAlreadyComplete)
            {
                continue;
            }

            ConditionProgress.CurrentCount = FMath::Min(
                ConditionProgress.CurrentCount + Amount,
                ConditionProgress.RequiredCount);
            bUpdatedQuest = true;
        }

        if (!bUpdatedQuest)
        {
            continue;
        }

        if (AreConditionsCompleted(QuestProgress))
        {
            QuestProgress.State = EPTQuestProgressState::Completed;
            OnQuestCompleted.Broadcast(QuestProgress.QuestID);
        }

        OnQuestProgressChanged.Broadcast(QuestProgress.QuestID, QuestProgress);
        bUpdatedAnyQuest = true;
    }

    if (bUpdatedAnyQuest)
    {
        PlayerState->ForceNetUpdate();
        SaveQuestPlayerState(PlayerState, TEXT("ProgressUpdated"));
    }

    return bUpdatedAnyQuest;
}

bool UPTQuestSubsystem::CompleteQuest(APTBasePlayerState* PlayerState, FName QuestID)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return false;
    }

    FPTQuestProgress* QuestProgress = FindQuestProgress(PlayerState, QuestID);
    if (QuestProgress == nullptr || QuestProgress->State != EPTQuestProgressState::InProgress)
    {
        return false;
    }

    QuestProgress->State = EPTQuestProgressState::Completed;
    PlayerState->ForceNetUpdate();
    OnQuestCompleted.Broadcast(QuestID);
    OnQuestProgressChanged.Broadcast(QuestID, *QuestProgress);
    OnQuestListChanged.Broadcast();
    SaveQuestPlayerState(PlayerState, TEXT("Completed"));
    return true;
}

bool UPTQuestSubsystem::RewardQuest(FName QuestID, APTBasePlayerState* RewardPlayerState)
{
    if (RewardPlayerState == nullptr || !RewardPlayerState->HasAuthority())
    {
        return false;
    }

    FPTQuestProgress* QuestProgress = FindQuestProgress(RewardPlayerState, QuestID);
    if (QuestProgress == nullptr || QuestProgress->State != EPTQuestProgressState::Completed)
    {
        return false;
    }

    const FPTQuestDataRow* QuestData = GetQuestData(QuestID);
    if (QuestData == nullptr || !GiveQuestRewards(*QuestData, RewardPlayerState))
    {
        return false;
    }

    QuestProgress->State = EPTQuestProgressState::Rewarded;
    RewardPlayerState->ForceNetUpdate();
    OnQuestProgressChanged.Broadcast(QuestID, *QuestProgress);
    OnQuestListChanged.Broadcast();
    SaveQuestPlayerState(RewardPlayerState, TEXT("Rewarded"));
    return true;
}

bool UPTQuestSubsystem::IsQuestCompleted(const APTBasePlayerState* PlayerState, FName QuestID) const
{
    const FPTQuestProgress* QuestProgress = GetQuestProgress(PlayerState, QuestID);
    return QuestProgress != nullptr &&
        (QuestProgress->State == EPTQuestProgressState::Completed ||
            QuestProgress->State == EPTQuestProgressState::Rewarded);
}

bool UPTQuestSubsystem::IsQuestRewarded(const APTBasePlayerState* PlayerState, FName QuestID) const
{
    const FPTQuestProgress* QuestProgress = GetQuestProgress(PlayerState, QuestID);
    return QuestProgress != nullptr && QuestProgress->State == EPTQuestProgressState::Rewarded;
}

bool UPTQuestSubsystem::ArePrerequisiteQuestsRewarded(
    const APTBasePlayerState* PlayerState,
    FName QuestID) const
{
    const FPTQuestDataRow* QuestData = GetQuestData(QuestID);
    if (PlayerState == nullptr || QuestData == nullptr)
    {
        return false;
    }

    for (FName PrerequisiteQuestID : QuestData->PrerequisiteQuestIDs)
    {
        if (!PrerequisiteQuestID.IsNone() &&
            !IsQuestRewarded(PlayerState, PrerequisiteQuestID))
        {
            return false;
        }
    }

    return true;
}

bool UPTQuestSubsystem::IsPlayerLevelRequirementMet(
    const APTBasePlayerState* PlayerState,
    FName QuestID) const
{
    const FPTQuestDataRow* QuestData = GetQuestData(QuestID);
    if (PlayerState == nullptr || QuestData == nullptr)
    {
        return false;
    }

    const int32 RequiredPlayerLevel = FMath::Max(QuestData->RequiredPlayerLevel, 1);
    return PlayerState->PlayerLevel >= RequiredPlayerLevel;
}

bool UPTQuestSubsystem::AreQuestRequirementsMet(
    const APTBasePlayerState* PlayerState,
    FName QuestID) const
{
    return ArePrerequisiteQuestsRewarded(PlayerState, QuestID) &&
        IsPlayerLevelRequirementMet(PlayerState, QuestID);
}

TArray<FPTQuestProgress> UPTQuestSubsystem::GetAcceptedQuestProgresses(const APTBasePlayerState* PlayerState) const
{
    return PlayerState != nullptr ? PlayerState->AcceptedQuests : TArray<FPTQuestProgress>();
}

void UPTQuestSubsystem::SetAcceptedQuestProgresses(
    APTBasePlayerState* PlayerState,
    const TArray<FPTQuestProgress>& InQuestProgresses)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return;
    }

    PlayerState->AcceptedQuests.Empty();
    TSet<FName> LoadedQuestIDs;
    for (const FPTQuestProgress& QuestProgress : InQuestProgresses)
    {
        if (QuestProgress.QuestID.IsNone() || LoadedQuestIDs.Contains(QuestProgress.QuestID))
        {
            continue;
        }

        FPTQuestProgress RestoredQuestProgress = QuestProgress;
        for (FPTQuestConditionProgress& ConditionProgress : RestoredQuestProgress.Conditions)
        {
            ConditionProgress.RequiredCount = FMath::Max(ConditionProgress.RequiredCount, 1);
            ConditionProgress.CurrentCount = FMath::Clamp(
                ConditionProgress.CurrentCount,
                0,
                ConditionProgress.RequiredCount);
        }

        LoadedQuestIDs.Add(RestoredQuestProgress.QuestID);
        PlayerState->AcceptedQuests.Add(MoveTemp(RestoredQuestProgress));
    }

    PlayerState->ForceNetUpdate();
    OnQuestListChanged.Broadcast();
}

void UPTQuestSubsystem::ClearAcceptedQuestProgresses(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return;
    }

    PlayerState->AcceptedQuests.Empty();
    PlayerState->ForceNetUpdate();
}

void UPTQuestSubsystem::BroadcastQuestListChanged()
{
    OnQuestListChanged.Broadcast();
}

FPTQuestProgress* UPTQuestSubsystem::FindQuestProgress(APTBasePlayerState* PlayerState, FName QuestID) const
{
    if (PlayerState == nullptr || QuestID.IsNone())
    {
        return nullptr;
    }

    return PlayerState->AcceptedQuests.FindByPredicate([QuestID](const FPTQuestProgress& QuestProgress)
        {
            return QuestProgress.QuestID == QuestID;
        });
}

void UPTQuestSubsystem::SaveQuestPlayerState(
    APTBasePlayerState* PlayerState,
    const TCHAR* SaveReason) const
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    const bool bSaved = SaveSubsystem != nullptr && SaveSubsystem->SavePlayer(PlayerState);

    if (bSaved)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("[QuestSave] Saved. Reason=%s Player=%s QuestCount=%d"),
            SaveReason,
            *PlayerState->GetPlayerName(),
            PlayerState->AcceptedQuests.Num());
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[QuestSave] Failed. Reason=%s Player=%s QuestCount=%d"),
            SaveReason,
            *PlayerState->GetPlayerName(),
            PlayerState->AcceptedQuests.Num());
    }
}

FPTQuestProgress UPTQuestSubsystem::MakeQuestProgress(const FPTQuestDataRow& QuestData) const
{
    FPTQuestProgress QuestProgress;
    QuestProgress.QuestID = QuestData.QuestID;
    QuestProgress.State = EPTQuestProgressState::InProgress;

    for (const FPTQuestCondition& Condition : QuestData.Conditions)
    {
        FPTQuestConditionProgress ConditionProgress;
        ConditionProgress.ConditionType = Condition.ConditionType;
        ConditionProgress.TargetID = Condition.TargetID;
        ConditionProgress.CurrentCount = 0;
        ConditionProgress.RequiredCount = FMath::Max(Condition.RequiredCount, 1);

        QuestProgress.Conditions.Add(ConditionProgress);
    }

    return QuestProgress;
}

bool UPTQuestSubsystem::AreConditionsCompleted(const FPTQuestProgress& QuestProgress) const
{
    if (QuestProgress.Conditions.IsEmpty())
    {
        return false;
    }

    for (const FPTQuestConditionProgress& ConditionProgress : QuestProgress.Conditions)
    {
        if (ConditionProgress.CurrentCount < ConditionProgress.RequiredCount)
        {
            return false;
        }
    }

    return true;
}

UPTInventoryComponent* UPTQuestSubsystem::GetRewardPlayerInventory(APTBasePlayerState* RewardPlayerState) const
{
    AController* Controller = RewardPlayerState != nullptr ? Cast<AController>(RewardPlayerState->GetOwner()) : nullptr;
    APTPlayerCharacter* PlayerCharacter = Controller != nullptr ? Cast<APTPlayerCharacter>(Controller->GetPawn()) : nullptr;
    return PlayerCharacter != nullptr ? PlayerCharacter->GetInventoryComponent() : nullptr;
}

bool UPTQuestSubsystem::HasRequiredCollectItems(
    const FPTQuestDataRow& QuestData,
    APTBasePlayerState* RewardPlayerState) const
{
    TMap<FName, int32> RequiredItemCounts;
    for (const FPTQuestCondition& Condition : QuestData.Conditions)
    {
        if (Condition.ConditionType != EPTQuestConditionType::CollectItem || Condition.TargetID.IsNone())
        {
            continue;
        }

        RequiredItemCounts.FindOrAdd(Condition.TargetID) += FMath::Max(Condition.RequiredCount, 1);
    }

    if (RequiredItemCounts.IsEmpty())
    {
        return true;
    }

    UPTInventoryComponent* InventoryComponent = GetRewardPlayerInventory(RewardPlayerState);
    if (InventoryComponent == nullptr)
    {
        return false;
    }

    for (const TPair<FName, int32>& RequiredItemCount : RequiredItemCounts)
    {
        if (InventoryComponent->GetItemCount(RequiredItemCount.Key) < RequiredItemCount.Value)
        {
            return false;
        }
    }

    return true;
}

bool UPTQuestSubsystem::ConsumeRequiredCollectItems(
    const FPTQuestDataRow& QuestData,
    APTBasePlayerState* RewardPlayerState) const
{
    TMap<FName, int32> RequiredItemCounts;
    for (const FPTQuestCondition& Condition : QuestData.Conditions)
    {
        if (Condition.ConditionType != EPTQuestConditionType::CollectItem || Condition.TargetID.IsNone())
        {
            continue;
        }

        RequiredItemCounts.FindOrAdd(Condition.TargetID) += FMath::Max(Condition.RequiredCount, 1);
    }

    if (RequiredItemCounts.IsEmpty())
    {
        return true;
    }

    UPTInventoryComponent* InventoryComponent = GetRewardPlayerInventory(RewardPlayerState);
    if (InventoryComponent == nullptr)
    {
        return false;
    }

    for (const TPair<FName, int32>& RequiredItemCount : RequiredItemCounts)
    {
        if (!InventoryComponent->RemoveItem(RequiredItemCount.Key, RequiredItemCount.Value))
        {
            return false;
        }
    }

    return true;
}

bool UPTQuestSubsystem::GiveQuestRewards(const FPTQuestDataRow& QuestData, APTBasePlayerState* RewardPlayerState) const
{
    if (RewardPlayerState == nullptr || !RewardPlayerState->HasAuthority())
    {
        return false;
    }

    if (!HasRequiredCollectItems(QuestData, RewardPlayerState))
    {
        return false;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return false;
    }

    UPTInventoryComponent* InventoryComponent = GetRewardPlayerInventory(RewardPlayerState);
    TArray<FItemData> RewardItems;
    if (!QuestData.RewardItemIDs.IsEmpty())
    {
        if (InventoryComponent == nullptr)
        {
            return false;
        }

        UPTItemSubsystem* ItemSubsystem = GameInstance->GetSubsystem<UPTItemSubsystem>();
        if (ItemSubsystem == nullptr)
        {
            return false;
        }

        TArray<FInventorySlot> SimulatedSlots = InventoryComponent->GetInventorySlots();
        for (FName RewardItemID : QuestData.RewardItemIDs)
        {
            if (RewardItemID.IsNone())
            {
                return false;
            }

            const FItemData* RewardItemData = ItemSubsystem->GetItemData(RewardItemID);
            if (RewardItemData == nullptr)
            {
                UE_LOG(LogTemp, Warning, TEXT("[QuestReward] Reward item data was not found: %s"),
                    *RewardItemID.ToString());
                return false;
            }

            bool bCanPlaceRewardItem = false;
            if (RewardItemData->Item_Category == EItemCategory::Consumable)
            {
                for (FInventorySlot& SimulatedSlot : SimulatedSlots)
                {
                    if (!SimulatedSlot.IsEmpty() &&
                        SimulatedSlot.ItemData.Item_Category == EItemCategory::Consumable &&
                        SimulatedSlot.ItemData.Item_ID == RewardItemData->Item_ID)
                    {
                        ++SimulatedSlot.Quantity;
                        bCanPlaceRewardItem = true;
                        break;
                    }
                }
            }

            if (!bCanPlaceRewardItem)
            {
                for (FInventorySlot& SimulatedSlot : SimulatedSlots)
                {
                    if (SimulatedSlot.IsEmpty())
                    {
                        SimulatedSlot.ItemData = *RewardItemData;
                        SimulatedSlot.Quantity = 1;
                        SimulatedSlot.ItemIconAsset = RewardItemData->Item_Icon;
                        bCanPlaceRewardItem = true;
                        break;
                    }
                }
            }

            if (!bCanPlaceRewardItem)
            {
                UE_LOG(LogTemp, Warning, TEXT("[QuestReward] Not enough inventory space for reward item: %s"),
                    *RewardItemID.ToString());
                return false;
            }

            RewardItems.Add(*RewardItemData);
        }
    }

    UPTEconomySubsystem* EconomySubsystem = nullptr;
    if (QuestData.RewardGold > 0)
    {
        EconomySubsystem = GameInstance->GetSubsystem<UPTEconomySubsystem>();
        if (EconomySubsystem == nullptr)
        {
            return false;
        }
    }

    UPTPlayerLevelSubsystem* PlayerLevelSubsystem = nullptr;
    if (QuestData.RewardExp > 0)
    {
        PlayerLevelSubsystem = GameInstance->GetSubsystem<UPTPlayerLevelSubsystem>();
        if (PlayerLevelSubsystem == nullptr)
        {
            return false;
        }
    }

    if (!ConsumeRequiredCollectItems(QuestData, RewardPlayerState))
    {
        return false;
    }

    for (const FItemData& RewardItem : RewardItems)
    {
        if (InventoryComponent == nullptr || !InventoryComponent->TryAddItem(RewardItem, 1))
        {
            UE_LOG(LogTemp, Error, TEXT("[QuestReward] Failed to grant prevalidated reward item: %s"),
                *RewardItem.Item_ID.ToString());
            return false;
        }
    }

    if (QuestData.RewardGold > 0)
    {
        EconomySubsystem->AddGold(RewardPlayerState, QuestData.RewardGold);
    }

    if (QuestData.RewardExp > 0)
    {
        PlayerLevelSubsystem->AddExp(RewardPlayerState, QuestData.RewardExp);
    }

    return true;
}
