#include "PTItemSubsystem.h"

#include "Engine/DataTable.h"

void UPTItemSubsystem::SetItemDataTable(UDataTable* InItemDataTable)
{
    ItemDataTable = InItemDataTable;
    RebuildItemDataMap();
}

void UPTItemSubsystem::RebuildItemDataMap()
{
    ItemDataMap.Empty();

    if (ItemDataTable == nullptr)
    {
        return;
    }

    TArray<FItemData*> ItemRows;
    ItemDataTable->GetAllRows<FItemData>(TEXT("Item Data Map"), ItemRows);

    for (const FItemData* ItemRow : ItemRows)
    {
        if (ItemRow == nullptr || ItemRow->Item_ID.IsNone())
        {
            continue;
        }

        ItemDataMap.Add(ItemRow->Item_ID, *ItemRow);
    }
}

const FItemData* UPTItemSubsystem::GetItemData(FName ItemID) const
{
    if (ItemID.IsNone())
    {
        return nullptr;
    }

    return ItemDataMap.Find(ItemID);
}

bool UPTItemSubsystem::HasItemData(FName ItemID) const
{
    return GetItemData(ItemID) != nullptr;
}
