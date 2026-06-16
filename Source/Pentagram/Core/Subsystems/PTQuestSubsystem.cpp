#include "PTQuestSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "PTEconomySubsystem.h"
#include "PTPlayerLevelSubsystem.h"

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

bool UPTQuestSubsystem::AcceptQuest(FName QuestID)
{
    if (QuestID.IsNone() || AcceptedQuestProgressMap.Contains(QuestID))
    {
        return false;
    }

    const FPTQuestDataRow* QuestData = GetQuestData(QuestID);
    if (QuestData == nullptr)
    {
        return false;
    }

    AcceptedQuestProgressMap.Add(QuestID, MakeQuestProgress(*QuestData));
    OnQuestAccepted.Broadcast(QuestID);
    return true;
}

bool UPTQuestSubsystem::HasAcceptedQuest(FName QuestID) const
{
    return !QuestID.IsNone() && AcceptedQuestProgressMap.Contains(QuestID);
}

const FPTQuestProgress* UPTQuestSubsystem::GetQuestProgress(FName QuestID) const
{
    if (QuestID.IsNone())
    {
        return nullptr;
    }

    return AcceptedQuestProgressMap.Find(QuestID);
}

bool UPTQuestSubsystem::UpdateQuestProgress(EPTQuestConditionType ConditionType, FName TargetID, int32 Amount)
{
    if (ConditionType == EPTQuestConditionType::None || Amount <= 0)
    {
        return false;
    }

    bool bUpdatedAnyQuest = false;

    for (auto& AcceptedQuest : AcceptedQuestProgressMap)
    {
        FPTQuestProgress& QuestProgress = AcceptedQuest.Value;
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

    return bUpdatedAnyQuest;
}

bool UPTQuestSubsystem::CompleteQuest(FName QuestID)
{
    FPTQuestProgress* QuestProgress = AcceptedQuestProgressMap.Find(QuestID);
    if (QuestProgress == nullptr || QuestProgress->State != EPTQuestProgressState::InProgress)
    {
        return false;
    }

    QuestProgress->State = EPTQuestProgressState::Completed;
    OnQuestCompleted.Broadcast(QuestID);
    OnQuestProgressChanged.Broadcast(QuestID, *QuestProgress);
    return true;
}

bool UPTQuestSubsystem::RewardQuest(FName QuestID, APTBasePlayerState* RewardPlayerState)
{
    FPTQuestProgress* QuestProgress = AcceptedQuestProgressMap.Find(QuestID);
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
    OnQuestProgressChanged.Broadcast(QuestID, *QuestProgress);
    return true;
}

bool UPTQuestSubsystem::IsQuestCompleted(FName QuestID) const
{
    const FPTQuestProgress* QuestProgress = GetQuestProgress(QuestID);
    return QuestProgress != nullptr &&
        (QuestProgress->State == EPTQuestProgressState::Completed ||
            QuestProgress->State == EPTQuestProgressState::Rewarded);
}

bool UPTQuestSubsystem::IsQuestRewarded(FName QuestID) const
{
    const FPTQuestProgress* QuestProgress = GetQuestProgress(QuestID);
    return QuestProgress != nullptr && QuestProgress->State == EPTQuestProgressState::Rewarded;
}

TArray<FPTQuestProgress> UPTQuestSubsystem::GetAcceptedQuestProgresses() const
{
    TArray<FPTQuestProgress> QuestProgresses;
    AcceptedQuestProgressMap.GenerateValueArray(QuestProgresses);
    return QuestProgresses;
}

void UPTQuestSubsystem::SetAcceptedQuestProgresses(const TArray<FPTQuestProgress>& InQuestProgresses)
{
    AcceptedQuestProgressMap.Empty();

    for (const FPTQuestProgress& QuestProgress : InQuestProgresses)
    {
        if (QuestProgress.QuestID.IsNone())
        {
            continue;
        }

        AcceptedQuestProgressMap.Add(QuestProgress.QuestID, QuestProgress);
    }
}

void UPTQuestSubsystem::ClearAcceptedQuestProgresses()
{
    AcceptedQuestProgressMap.Empty();
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

bool UPTQuestSubsystem::GiveQuestRewards(const FPTQuestDataRow& QuestData, APTBasePlayerState* RewardPlayerState) const
{
    if (RewardPlayerState == nullptr || !RewardPlayerState->HasAuthority())
    {
        return false;
    }

    if (!QuestData.RewardItemIDs.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[QuestReward] %s quest item rewards are skipped because item reward data is not wired yet."),
            *QuestData.QuestID.ToString());
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return false;
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
