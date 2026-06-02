#include "PTQuestSubsystem.h"

#include "Engine/DataTable.h"

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
