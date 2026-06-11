#pragma once

#include "CoreMinimal.h"
#include "Character/NPC/PTNPCCharacter.h"
#include "PTQuestNPCCharacter.generated.h"

class APawn;
class UPTNPCDialogueWidget;

UCLASS()
class PENTAGRAM_API APTQuestNPCCharacter : public APTNPCCharacter
{
    GENERATED_BODY()

public:
    virtual void Interact_Implementation(AActor* InteractorCharacter) override;

    UFUNCTION(Server, Reliable)
    void ServerAcceptQuest(FName QuestID);

    UFUNCTION(Server, Reliable)
    void ServerRewardQuest(FName QuestID);

    const TArray<FName>& GetQuestIDs() const;

protected:
    UFUNCTION(NetMulticast, Reliable)
    void MulticastOpenQuestDialogue(APawn* InteractPawn);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Quest")
    TArray<FName> QuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Quest")
    TSubclassOf<UPTNPCDialogueWidget> QuestDialogueWidgetClass;
};
