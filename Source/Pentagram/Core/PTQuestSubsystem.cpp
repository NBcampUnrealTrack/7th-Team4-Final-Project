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
