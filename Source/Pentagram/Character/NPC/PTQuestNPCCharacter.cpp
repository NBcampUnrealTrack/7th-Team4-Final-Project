#include "Character/NPC/PTQuestNPCCharacter.h"

#include "Character/Player/PTPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

void APTQuestNPCCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTQuestNPCCharacter, QuestIDs);
    DOREPLIFETIME(APTQuestNPCCharacter, QuestDialogueWidgetClass);
}

void APTQuestNPCCharacter::Interact_Implementation(AActor* InteractorCharacter)
{
    Super::Interact_Implementation(InteractorCharacter);

    if (!HasAuthority() || InteractorCharacter == nullptr)
    {
        return;
    }

    APawn* InteractPawn = Cast<APawn>(InteractorCharacter);
    if (InteractPawn == nullptr)
    {
        return;
    }

    APTPlayerController* InteractPlayerController =
        Cast<APTPlayerController>(InteractPawn->GetController());
    if (InteractPlayerController == nullptr)
    {
        return;
    }

    InteractPlayerController->Client_OpenQuestDialogue(this, QuestDialogueWidgetClass);
}

const TArray<FName>& APTQuestNPCCharacter::GetQuestIDs() const
{
    return QuestIDs;
}
