#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTNPCDialogueWidget.generated.h"

class APTNPCCharacter;

UCLASS()
class PENTAGRAM_API UPTNPCDialogueWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void SetupDialogue(APTNPCCharacter* InNPC);

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void SelectQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void RequestAcceptQuest();

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void RequestRewardQuest();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "PT|NPC")
    TObjectPtr<APTNPCCharacter> TargetNPC;

    UPROPERTY(BlueprintReadOnly, Category = "PT|NPC")
    FName SelectedQuestID = NAME_None;
};
