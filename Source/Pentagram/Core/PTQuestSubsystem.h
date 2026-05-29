#pragma once

#include "CoreMinimal.h"
#include "PTQuestDataRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTQuestSubsystem.generated.h"

class UDataTable;

UCLASS()
class PENTAGRAM_API UPTQuestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    void SetQuestDataTable(UDataTable* InQuestDataTable);
    void RebuildQuestDataMap();

    const FPTQuestDataRow* GetQuestData(FName QuestID) const;
    bool HasQuestData(FName QuestID) const;

private:
    UPROPERTY(EditDefaultsOnly, Category = "PT|Quest")
    TObjectPtr<UDataTable> QuestDataTable;

    TMap<FName, FPTQuestDataRow> QuestDataMap;
};
