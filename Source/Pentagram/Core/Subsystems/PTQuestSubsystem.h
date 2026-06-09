#pragma once

#include "CoreMinimal.h"
#include "Core/PTQuestDataRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTQuestSubsystem.generated.h"

class UDataTable;

DECLARE_MULTICAST_DELEGATE_OneParam(FPTNativeOnQuestAccepted, FName);
DECLARE_MULTICAST_DELEGATE_OneParam(FPTNativeOnQuestCompleted, FName);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPTNativeOnQuestProgressChanged, FName, const FPTQuestProgress&);

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

    bool AcceptQuest(FName QuestID);
    bool HasAcceptedQuest(FName QuestID) const;
    const FPTQuestProgress* GetQuestProgress(FName QuestID) const;
    bool UpdateQuestProgress(EPTQuestConditionType ConditionType, FName TargetID, int32 Amount = 1);
    bool CompleteQuest(FName QuestID);
    bool RewardQuest(FName QuestID);
    bool IsQuestCompleted(FName QuestID) const;
    bool IsQuestRewarded(FName QuestID) const;
    TArray<FPTQuestProgress> GetAcceptedQuestProgresses() const;
    void SetAcceptedQuestProgresses(const TArray<FPTQuestProgress>& InQuestProgresses);
    void ClearAcceptedQuestProgresses();

private:
    FPTQuestProgress MakeQuestProgress(const FPTQuestDataRow& QuestData) const;
    bool AreConditionsCompleted(const FPTQuestProgress& QuestProgress) const;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Quest")
    TObjectPtr<UDataTable> QuestDataTable;

    TMap<FName, FPTQuestDataRow> QuestDataMap;
    TMap<FName, FPTQuestProgress> AcceptedQuestProgressMap;

public:
    FPTNativeOnQuestAccepted OnQuestAccepted;
    FPTNativeOnQuestCompleted OnQuestCompleted;
    FPTNativeOnQuestProgressChanged OnQuestProgressChanged;
};
