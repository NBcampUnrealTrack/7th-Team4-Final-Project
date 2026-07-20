#include "PTNPCDialogueWidget.h"

#include "Character/NPC/PTQuestNPCCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "InputCoreTypes.h"
#include "UI/Widget/NPC/PTQuestListEntryWidget.h"

void UPTNPCDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BindQuestDelegates();

    if (AcceptButton != nullptr)
    {
        AcceptButton->OnClicked.AddUniqueDynamic(this, &UPTNPCDialogueWidget::HandleQuestActionButtonClicked);
    }
}

void UPTNPCDialogueWidget::NativeDestruct()
{
    if (AcceptButton != nullptr)
    {
        AcceptButton->OnClicked.RemoveDynamic(this, &UPTNPCDialogueWidget::HandleQuestActionButtonClicked);
    }

    UnbindQuestDelegates();

    Super::NativeDestruct();
}

void UPTNPCDialogueWidget::NativeOnDeactivated()
{
    if (APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PlayerController->RestoreGameplayInput();
    }

    Super::NativeOnDeactivated();
}

UPTQuestSubsystem* UPTNPCDialogueWidget::ResolveQuestSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UPTQuestSubsystem>() : nullptr;
}

APTBasePlayerState* UPTNPCDialogueWidget::ResolveOwningPlayerState() const
{
    APlayerController* PlayerController = GetOwningPlayer();
    return PlayerController != nullptr ? PlayerController->GetPlayerState<APTBasePlayerState>() : nullptr;
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
    SetIsFocusable(true);
    SetFocus();

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

    UPTQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = ResolveOwningPlayerState();
    if (PlayerState == nullptr)
    {
        return;
    }

    for (FName QuestID : TargetNPC->GetQuestIDs())
    {
        if (QuestSubsystem->IsQuestRewarded(PlayerState, QuestID))
        {
            continue;
        }

        const FPTQuestDataRow* QuestData = QuestSubsystem->GetQuestData(QuestID);
        if (QuestData == nullptr)
        {
            continue;
        }

        if (!QuestSubsystem->HasAcceptedQuest(PlayerState, QuestID) &&
            !QuestSubsystem->AreQuestRequirementsMet(PlayerState, QuestID))
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

    UPTQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = ResolveOwningPlayerState();
    if (PlayerState == nullptr)
    {
        return;
    }

    for (const FPTQuestProgress& QuestProgress : QuestSubsystem->GetAcceptedQuestProgresses(PlayerState))
    {
        if (QuestProgress.State == EPTQuestProgressState::Rewarded)
        {
            continue;
        }

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
        Txt_Objective->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UPTNPCDialogueWidget::RefreshQuestText()
{
    if (SelectedQuestID.IsNone())
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
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
        APTBasePlayerState* PlayerState = ResolveOwningPlayerState();
        const FPTQuestProgress* QuestProgress =
            QuestSubsystem->GetQuestProgress(PlayerState, SelectedQuestID);

        if (QuestProgress != nullptr)
        {
            for (int32 ConditionIndex = 0; ConditionIndex < QuestData->Conditions.Num(); ++ConditionIndex)
            {
                const FPTQuestCondition& Condition = QuestData->Conditions[ConditionIndex];
                if (Condition.ObjectiveText.IsEmpty())
                {
                    continue;
                }

                const int32 RequiredCount = FMath::Max(Condition.RequiredCount, 1);
                const int32 CurrentCount = QuestProgress->Conditions.IsValidIndex(ConditionIndex)
                    ? QuestProgress->Conditions[ConditionIndex].CurrentCount
                    : 0;
                const FText ConditionText = FText::Format(
                    NSLOCTEXT("PTQuest", "ObjectiveProgress", "{0} ({1}/{2})"),
                    Condition.ObjectiveText,
                    FText::AsNumber(CurrentCount),
                    FText::AsNumber(RequiredCount));

                if (!ObjectiveText.IsEmpty())
                {
                    ObjectiveText = FText::Format(
                        NSLOCTEXT("PTQuest", "ObjectiveList", "{0}\n{1}"),
                        ObjectiveText,
                        ConditionText);
                }
                else
                {
                    ObjectiveText = ConditionText;
                }
            }
        }

        Txt_Objective->SetText(ObjectiveText);
        Txt_Objective->SetVisibility(
            ObjectiveText.IsEmpty()
                ? ESlateVisibility::Collapsed
                : ESlateVisibility::HitTestInvisible);
    }
}

void UPTNPCDialogueWidget::RefreshQuestActionButtons()
{
    bool bShowActionButton = false;
    bool bUseRewardAction = false;

    if (TargetNPC != nullptr && !SelectedQuestID.IsNone())
    {
        UPTQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
        APTBasePlayerState* PlayerState = ResolveOwningPlayerState();
        if (QuestSubsystem != nullptr && PlayerState != nullptr)
        {
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
    }

    if (Txt_AcceptButtonText != nullptr)
    {
        Txt_AcceptButtonText->SetText(
            bUseRewardAction
                ? NSLOCTEXT("PTQuest", "RewardQuestButton", "보상 받기")
                : NSLOCTEXT("PTQuest", "AcceptQuestButton", "수락"));
    }
}

void UPTNPCDialogueWidget::HandleQuestActionButtonClicked()
{
    if (TargetNPC == nullptr || SelectedQuestID.IsNone())
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = ResolveOwningPlayerState();
    if (PlayerState == nullptr)
    {
        return;
    }

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
    UPTQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
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
    UPTQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
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
    if (!SelectedQuestID.IsNone())
    {
        UPTQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
        APTBasePlayerState* PlayerState = ResolveOwningPlayerState();
        if (QuestSubsystem != nullptr && QuestSubsystem->IsQuestRewarded(PlayerState, SelectedQuestID))
        {
            SelectedQuestID = NAME_None;
            ClearQuestText();
        }
    }

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

FReply UPTNPCDialogueWidget::NativeOnPreviewKeyDown(
    const FGeometry& InGeometry,
    const FKeyEvent& InKeyEvent)
{
    if (TargetNPC == nullptr && InKeyEvent.GetKey() == EKeys::J)
    {
        CloseDialogue();
        return FReply::Handled();
    }

    return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}
