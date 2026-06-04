#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PTQuestDataRow.generated.h"

UENUM(BlueprintType)
enum class EPTQuestConditionType : uint8
{
    None        UMETA(DisplayName = "None"),
    KillMonster UMETA(DisplayName = "Kill Monster"),
    CollectItem UMETA(DisplayName = "Collect Item"),
    TalkToNPC   UMETA(DisplayName = "Talk To NPC"),
    ReachArea   UMETA(DisplayName = "Reach Area")
};

USTRUCT(BlueprintType)
struct PENTAGRAM_API FPTQuestCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    EPTQuestConditionType ConditionType = EPTQuestConditionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FName TargetID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest", meta = (ClampMin = "1"))
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FText ObjectiveText;
};

USTRUCT(BlueprintType)
struct PENTAGRAM_API FPTQuestDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FName QuestID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FText QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    TArray<FPTQuestCondition> Conditions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest|Reward", meta = (ClampMin = "0"))
    int32 RewardGold = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest|Reward", meta = (ClampMin = "0"))
    int32 RewardExp = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest|Reward")
    TArray<FName> RewardItemIDs;
};

UENUM(BlueprintType)
enum class EPTQuestProgressState : uint8
{
    InProgress UMETA(DisplayName = "In Progress"),
    Completed  UMETA(DisplayName = "Completed"),
    Rewarded   UMETA(DisplayName = "Rewarded")
};

USTRUCT(BlueprintType)
struct PENTAGRAM_API FPTQuestConditionProgress
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    EPTQuestConditionType ConditionType = EPTQuestConditionType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FName TargetID = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    int32 CurrentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    int32 RequiredCount = 1;
};

USTRUCT(BlueprintType)
struct PENTAGRAM_API FPTQuestProgress
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FName QuestID = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    EPTQuestProgressState State = EPTQuestProgressState::InProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    TArray<FPTQuestConditionProgress> Conditions;
};
