#include "PTNPCDialogueWidget.h"

#include "Character/NPC/PTQuestNPCCharacter.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "UI/Widget/NPC/PTQuestListEntryWidget.h"

void UPTNPCDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (AcceptButton != nullptr)
    {
        AcceptButton->OnClicked().AddUObject(this, &UPTNPCDialogueWidget::RequestAcceptQuest);
    }
}

void UPTNPCDialogueWidget::NativeDestruct()
{
    if (AcceptButton != nullptr)
    {
        AcceptButton->OnClicked().RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UPTNPCDialogueWidget::SetupDialogue(APTQuestNPCCharacter* InNPC)
{
    TargetNPC = InNPC;
    SelectedQuestID = NAME_None;

    if (AcceptButton != nullptr)
    {
        AcceptButton->SetVisibility(ESlateVisibility::Visible);
    }

    BuildQuestList();
    ClearQuestText();
}

void UPTNPCDialogueWidget::SetupQuestJournal()
{
    TargetNPC = nullptr;
    SelectedQuestID = NAME_None;

    if (AcceptButton != nullptr)
    {
        AcceptButton->SetVisibility(ESlateVisibility::Collapsed);
    }

    BuildAcceptedQuestList();
    ClearQuestText();
}

void UPTNPCDialogueWidget::SelectQuest(FName QuestID)
{
    SelectedQuestID = QuestID;
    RefreshQuestText();
}

void UPTNPCDialogueWidget::BuildQuestList()
{
    if (QuestList == nullptr || QuestEntryWidgetClass == nullptr || TargetNPC == nullptr)
    {
        return;
    }

    QuestList->ClearChildren();

    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    for (FName QuestID : TargetNPC->GetQuestIDs())
    {
        const FPTQuestDataRow* QuestData = QuestSubsystem->GetQuestData(QuestID);
        if (QuestData == nullptr)
        {
            continue;
        }

        UPTQuestListEntryWidget* QuestEntry = CreateWidget<UPTQuestListEntryWidget>(
            GetOwningPlayer(),
            QuestEntryWidgetClass);
        if (QuestEntry == nullptr)
        {
            continue;
        }

        QuestEntry->SetupQuestEntry(QuestID, FText::FromName(QuestID));
        QuestEntry->OnQuestEntryClicked.AddUObject(this, &UPTNPCDialogueWidget::OnQuestEntryClicked);
        QuestList->AddChildToVerticalBox(QuestEntry);
    }
}

void UPTNPCDialogueWidget::BuildAcceptedQuestList()
{
    if (QuestList == nullptr || QuestEntryWidgetClass == nullptr)
    {
        return;
    }

    QuestList->ClearChildren();

    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    for (const FPTQuestProgress& QuestProgress : QuestSubsystem->GetAcceptedQuestProgresses())
    {
        const FPTQuestDataRow* QuestData = QuestSubsystem->GetQuestData(QuestProgress.QuestID);
        if (QuestData == nullptr)
        {
            continue;
        }

        UPTQuestListEntryWidget* QuestEntry = CreateWidget<UPTQuestListEntryWidget>(
            GetOwningPlayer(),
            QuestEntryWidgetClass);
        if (QuestEntry == nullptr)
        {
            continue;
        }

        QuestEntry->SetupQuestEntry(QuestProgress.QuestID, FText::FromName(QuestProgress.QuestID));
        QuestEntry->OnQuestEntryClicked.AddUObject(this, &UPTNPCDialogueWidget::OnQuestEntryClicked);
        QuestList->AddChildToVerticalBox(QuestEntry);
    }
}

void UPTNPCDialogueWidget::OnQuestEntryClicked(FName QuestID)
{
    SelectQuest(QuestID);
}

void UPTNPCDialogueWidget::ClearQuestText()
{
    if (Txt_QuestName != nullptr)
    {
        Txt_QuestName->SetText(FText::GetEmpty());
    }

    if (Txt_Description != nullptr)
    {
        Txt_Description->SetText(FText::GetEmpty());
    }

    if (Txt_Objective != nullptr)
    {
        Txt_Objective->SetText(FText::GetEmpty());
    }
}

void UPTNPCDialogueWidget::RefreshQuestText()
{
    if (SelectedQuestID.IsNone())
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    const FPTQuestDataRow* QuestData = QuestSubsystem->GetQuestData(SelectedQuestID);
    if (QuestData == nullptr)
    {
        return;
    }

    if (Txt_QuestName != nullptr)
    {
        Txt_QuestName->SetText(QuestData->QuestName);
    }

    if (Txt_Description != nullptr)
    {
        Txt_Description->SetText(QuestData->Description);
    }

    if (Txt_Objective != nullptr)
    {
        FText ObjectiveText;
        for (const FPTQuestCondition& Condition : QuestData->Conditions)
        {
            if (Condition.ObjectiveText.IsEmpty())
            {
                continue;
            }

            if (!ObjectiveText.IsEmpty())
            {
                ObjectiveText = FText::Format(
                    NSLOCTEXT("PTQuest", "ObjectiveList", "{0}\n{1}"),
                    ObjectiveText,
                    Condition.ObjectiveText);
            }
            else
            {
                ObjectiveText = Condition.ObjectiveText;
            }
        }

        Txt_Objective->SetText(ObjectiveText);
    }
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

void UPTNPCDialogueWidget::CloseDialogue()
{
    DeactivateWidget();
}

bool UPTNPCDialogueWidget::NativeOnHandleBackAction()
{
    bIsBackHandler = true;
    CloseDialogue();
    return true;
}
