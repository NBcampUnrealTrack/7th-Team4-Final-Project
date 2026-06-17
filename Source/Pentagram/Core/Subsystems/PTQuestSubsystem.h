#pragma once

#include "CoreMinimal.h"
#include "Core/PTQuestDataRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTQuestSubsystem.generated.h"

class UDataTable;
class APTBasePlayerState;

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

    bool AcceptQuest(APTBasePlayerState* PlayerState, FName QuestID);
    bool HasAcceptedQuest(const APTBasePlayerState* PlayerState, FName QuestID) const;
    const FPTQuestProgress* GetQuestProgress(const APTBasePlayerState* PlayerState, FName QuestID) const;
    bool UpdateQuestProgress(APTBasePlayerState* PlayerState, EPTQuestConditionType ConditionType, FName TargetID, int32 Amount = 1);
    bool CompleteQuest(APTBasePlayerState* PlayerState, FName QuestID);
    bool RewardQuest(FName QuestID, APTBasePlayerState* RewardPlayerState);
    bool IsQuestCompleted(const APTBasePlayerState* PlayerState, FName QuestID) const;
    bool IsQuestRewarded(const APTBasePlayerState* PlayerState, FName QuestID) const;
    TArray<FPTQuestProgress> GetAcceptedQuestProgresses(const APTBasePlayerState* PlayerState) const;
    void SetAcceptedQuestProgresses(APTBasePlayerState* PlayerState, const TArray<FPTQuestProgress>& InQuestProgresses);
    void ClearAcceptedQuestProgresses(APTBasePlayerState* PlayerState);

private:
    FPTQuestProgress MakeQuestProgress(const FPTQuestDataRow& QuestData) const;
    bool AreConditionsCompleted(const FPTQuestProgress& QuestProgress) const;
    bool GiveQuestRewards(const FPTQuestDataRow& QuestData, APTBasePlayerState* RewardPlayerState) const;
    FPTQuestProgress* FindQuestProgress(APTBasePlayerState* PlayerState, FName QuestID) const;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Quest")
    TObjectPtr<UDataTable> QuestDataTable;

    TMap<FName, FPTQuestDataRow> QuestDataMap;

public:
    FPTNativeOnQuestAccepted OnQuestAccepted;
    FPTNativeOnQuestCompleted OnQuestCompleted;
    FPTNativeOnQuestProgressChanged OnQuestProgressChanged;
};
