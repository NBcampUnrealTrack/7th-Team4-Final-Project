#pragma once

#include "CoreMinimal.h"
#include "Item/PTItemTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTItemSubsystem.generated.h"

class UDataTable;

UCLASS()
class PENTAGRAM_API UPTItemSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void SetItemDataTable(UDataTable* InItemDataTable);
    void RebuildItemDataMap();

    const FItemData* GetItemData(FName ItemID) const;
    bool HasItemData(FName ItemID) const;

private:
    UPROPERTY()
    TObjectPtr<UDataTable> ItemDataTable;

    TMap<FName, FItemData> ItemDataMap;
};
