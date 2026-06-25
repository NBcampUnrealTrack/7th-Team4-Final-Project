#include "UI/Widget/NPC/PTQuestListEntryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UPTQuestListEntryWidget::SetupQuestEntry(FName InQuestID, const FText& InQuestName)
{
    QuestID = InQuestID;
    if (Txt_QuestName != nullptr)
    {
        Txt_QuestName->SetText(InQuestName);
    }
}

void UPTQuestListEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_QuestEntry != nullptr)
    {
        Btn_QuestEntry->OnClicked.AddUniqueDynamic(this, &UPTQuestListEntryWidget::OnEntryClicked);
    }
}

void UPTQuestListEntryWidget::NativeDestruct()
{
    if (Btn_QuestEntry != nullptr)
    {
        Btn_QuestEntry->OnClicked.RemoveDynamic(this, &UPTQuestListEntryWidget::OnEntryClicked);
    }

    Super::NativeDestruct();
}

void UPTQuestListEntryWidget::OnEntryClicked()
{
    if (!QuestID.IsNone())
    {
        OnQuestEntryClicked.Broadcast(QuestID);
    }
}
