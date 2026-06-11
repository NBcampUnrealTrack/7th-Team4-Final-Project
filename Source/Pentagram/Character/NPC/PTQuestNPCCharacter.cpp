#include "Character/NPC/PTQuestNPCCharacter.h"

#include "Core/Subsystems/PTQuestSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Widget/NPC/PTNPCDialogueWidget.h"

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

    MulticastOpenQuestDialogue(InteractPawn);
}

void APTQuestNPCCharacter::MulticastOpenQuestDialogue_Implementation(APawn* InteractPawn)
{
    if (InteractPawn == nullptr || !InteractPawn->IsLocallyControlled() || QuestDialogueWidgetClass == nullptr)
    {
        return;
    }

    APlayerController* InteractPlayerController =
        Cast<APlayerController>(InteractPawn->GetController());
    if (InteractPlayerController == nullptr)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = InteractPlayerController->GetLocalPlayer();
    if (LocalPlayer == nullptr)
    {
        return;
    }

    UPTUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPTUIManagerSubsystem>();
    if (UIManager == nullptr)
    {
        return;
    }

    UPTNPCDialogueWidget* DialogueWidget = Cast<UPTNPCDialogueWidget>(
        UIManager->PushWidget(QuestDialogueWidgetClass, EPTUILayer::GameMenu));
    if (DialogueWidget != nullptr)
    {
        DialogueWidget->SetupDialogue(this);
    }
}

void APTQuestNPCCharacter::ServerAcceptQuest_Implementation(FName QuestID)
{
    if (QuestID.IsNone() || !QuestIDs.Contains(QuestID))
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->AcceptQuest(QuestID);
    }
}

void APTQuestNPCCharacter::ServerRewardQuest_Implementation(FName QuestID)
{
    if (QuestID.IsNone() || !QuestIDs.Contains(QuestID))
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->RewardQuest(QuestID);
    }
}

const TArray<FName>& APTQuestNPCCharacter::GetQuestIDs() const
{
    return QuestIDs;
}
