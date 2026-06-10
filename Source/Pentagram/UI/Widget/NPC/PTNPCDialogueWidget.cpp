#include "PTNPCDialogueWidget.h"

#include "Character/NPC/PTNPCCharacter.h"

void UPTNPCDialogueWidget::SetupDialogue(APTNPCCharacter* InNPC)
{
    TargetNPC = InNPC;

    SelectedQuestID = NAME_None;
    if (TargetNPC != nullptr && !TargetNPC->GetQuestIDs().IsEmpty())
    {
        SelectedQuestID = TargetNPC->GetQuestIDs()[0];
    }
}

void UPTNPCDialogueWidget::SelectQuest(FName QuestID)
{
    SelectedQuestID = QuestID;
}

void UPTNPCDialogueWidget::RequestAcceptQuest()
{
    if (TargetNPC == nullptr || SelectedQuestID.IsNone())
    {
        return;
    }

    TargetNPC->ServerAcceptQuest(SelectedQuestID);
}

void UPTNPCDialogueWidget::RequestRewardQuest()
{
    if (TargetNPC == nullptr || SelectedQuestID.IsNone())
    {
        return;
    }

    TargetNPC->ServerRewardQuest(SelectedQuestID);
}
