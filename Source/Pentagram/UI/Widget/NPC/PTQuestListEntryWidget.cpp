#include "UI/Widget/NPC/PTQuestListEntryWidget.h"

void UPTQuestListEntryWidget::SetupQuestEntry(FName InQuestID, const FText& InQuestName)
{
    QuestID = InQuestID;
    SetButtonText(InQuestName);
}

void UPTQuestListEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    OnClicked().AddUObject(this, &UPTQuestListEntryWidget::OnEntryClicked);
}

void UPTQuestListEntryWidget::NativeDestruct()
{
    OnClicked().RemoveAll(this);

    Super::NativeDestruct();
}

void UPTQuestListEntryWidget::OnEntryClicked()
{
    if (!QuestID.IsNone())
    {
        OnQuestEntryClicked.Broadcast(QuestID);
    }
}
