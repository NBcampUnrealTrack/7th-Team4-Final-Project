#include "PTNPCDialogueWidget.h"

#include "Character/NPC/PTQuestNPCCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerController.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "UI/Components/PTCommonButtonBase.h"
#include "UI/Widget/NPC/PTQuestListEntryWidget.h"

void UPTNPCDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BindQuestDelegates();

    if (AcceptButton != nullptr)
    {
        AcceptButton->OnClicked().AddUObject(this, &UPTNPCDialogueWidget::HandleQuestActionButtonClicked);
    }
}

void UPTNPCDialogueWidget::NativeDestruct()
{
    if (AcceptButton != nullptr)
    {
        AcceptButton->OnClicked().RemoveAll(this);
    }

    UnbindQuestDelegates();

    Super::NativeDestruct();
}

void UPTNPCDialogueWidget::SetupDialogue(APTQuestNPCCharacter* InNPC)
{
    TargetNPC = InNPC;
    SelectedQuestID = NAME_None;

    BuildQuestList();
    ClearQuestText();
    RefreshQuestActionButtons();
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
    RefreshQuestActionButtons();
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

        QuestEntry->SetupQuestEntry(QuestID, QuestData->QuestName);
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

    APTBasePlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<APTBasePlayerState>();

    for (const FPTQuestProgress& QuestProgress : QuestSubsystem->GetAcceptedQuestProgresses(PlayerState))
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

        QuestEntry->SetupQuestEntry(QuestProgress.QuestID, QuestData->QuestName);
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

void UPTNPCDialogueWidget::RefreshQuestActionButtons()
{
    bool bShowActionButton = false;
    bool bUseRewardAction = false;

    if (TargetNPC != nullptr && !SelectedQuestID.IsNone())
    {
        UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
        if (QuestSubsystem != nullptr)
        {
            APTBasePlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<APTBasePlayerState>();
            const bool bHasAcceptedQuest = QuestSubsystem->HasAcceptedQuest(PlayerState, SelectedQuestID);
            const bool bIsCompleted = QuestSubsystem->IsQuestCompleted(PlayerState, SelectedQuestID);
            const bool bIsRewarded = QuestSubsystem->IsQuestRewarded(PlayerState, SelectedQuestID);

            bUseRewardAction = bIsCompleted && !bIsRewarded;
            bShowActionButton = !bHasAcceptedQuest || bUseRewardAction;
        }
    }

    if (AcceptButton != nullptr)
    {
        AcceptButton->SetVisibility(bShowActionButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        AcceptButton->SetIsEnabled(bShowActionButton);

        if (UPTCommonButtonBase* QuestActionButton = Cast<UPTCommonButtonBase>(AcceptButton))
        {
            QuestActionButton->SetButtonText(
                bUseRewardAction
                    ? NSLOCTEXT("PTQuest", "RewardQuestButton", "보상 받기")
                    : NSLOCTEXT("PTQuest", "AcceptQuestButton", "수락"));
        }
    }
}

void UPTNPCDialogueWidget::HandleQuestActionButtonClicked()
{
    if (TargetNPC == nullptr || SelectedQuestID.IsNone())
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<APTBasePlayerState>();
    if (QuestSubsystem->IsQuestCompleted(PlayerState, SelectedQuestID) &&
        !QuestSubsystem->IsQuestRewarded(PlayerState, SelectedQuestID))
    {
        RequestRewardQuest();
        return;
    }

    if (!QuestSubsystem->HasAcceptedQuest(PlayerState, SelectedQuestID))
    {
        RequestAcceptQuest();
    }
}

void UPTNPCDialogueWidget::BindQuestDelegates()
{
    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    QuestSubsystem->OnQuestAccepted.RemoveAll(this);
    QuestSubsystem->OnQuestCompleted.RemoveAll(this);
    QuestSubsystem->OnQuestProgressChanged.RemoveAll(this);
    QuestSubsystem->OnQuestListChanged.RemoveAll(this);

    QuestSubsystem->OnQuestAccepted.AddUObject(this, &UPTNPCDialogueWidget::HandleQuestAccepted);
    QuestSubsystem->OnQuestCompleted.AddUObject(this, &UPTNPCDialogueWidget::HandleQuestCompleted);
    QuestSubsystem->OnQuestProgressChanged.AddUObject(this, &UPTNPCDialogueWidget::HandleQuestProgressChanged);
    QuestSubsystem->OnQuestListChanged.AddUObject(this, &UPTNPCDialogueWidget::HandleQuestListChanged);
}

void UPTNPCDialogueWidget::UnbindQuestDelegates()
{
    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    QuestSubsystem->OnQuestAccepted.RemoveAll(this);
    QuestSubsystem->OnQuestCompleted.RemoveAll(this);
    QuestSubsystem->OnQuestProgressChanged.RemoveAll(this);
    QuestSubsystem->OnQuestListChanged.RemoveAll(this);
}

void UPTNPCDialogueWidget::RefreshVisibleQuestList()
{
    if (TargetNPC != nullptr)
    {
        BuildQuestList();
    }
    else
    {
        BuildAcceptedQuestList();
    }

    RefreshQuestText();
    RefreshQuestActionButtons();
}

void UPTNPCDialogueWidget::HandleQuestAccepted(FName QuestID)
{
    RefreshVisibleQuestList();
}

void UPTNPCDialogueWidget::HandleQuestCompleted(FName QuestID)
{
    RefreshVisibleQuestList();
}

void UPTNPCDialogueWidget::HandleQuestProgressChanged(FName QuestID, const FPTQuestProgress& QuestProgress)
{
    if (SelectedQuestID == QuestID)
    {
        RefreshQuestText();
        RefreshQuestActionButtons();
    }
}

void UPTNPCDialogueWidget::HandleQuestListChanged()
{
    RefreshVisibleQuestList();
}

void UPTNPCDialogueWidget::RequestAcceptQuest()
{
    if (TargetNPC == nullptr || SelectedQuestID.IsNone())
    {
        return;
    }

    APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer());
    if (PlayerController != nullptr)
    {
        PlayerController->ServerAcceptQuest(TargetNPC, SelectedQuestID);
    }
}

void UPTNPCDialogueWidget::RequestRewardQuest()
{
    if (TargetNPC == nullptr || SelectedQuestID.IsNone())
    {
        return;
    }

    APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer());
    if (PlayerController != nullptr)
    {
        PlayerController->ServerRewardQuest(TargetNPC, SelectedQuestID);
    }
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
