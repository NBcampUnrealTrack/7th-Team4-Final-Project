// PTNotifyManagerWidget.cpp
#include "PTNotifyManagerWidget.h"
#include "PTNotifyWidget.h"

void UPTNotifyManagerWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Notify_Slot)
    {
        Notify_Slot->OnNotifyFinished.AddDynamic(
            this, &UPTNotifyManagerWidget::HandleCurrentFinished);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Notify][X] NativeOnInitialized - Notify_Slot is null! (BindWidget 이름 불일치 의심)"));
    }
}

void UPTNotifyManagerWidget::Enqueue(const FPTNotifyData& InData)
{
    UE_LOG(LogTemp, Warning, TEXT("[Notify][4] Enqueue called. bIsPlaying=%d"), bIsPlaying);

    NotifyQueue.Enqueue(InData);

    if (!bIsPlaying)
    {
        ProcessNext();
    }
}

void UPTNotifyManagerWidget::ProcessNext()
{
    FPTNotifyData NextData;
    if (NotifyQueue.Dequeue(NextData))
    {
        bIsPlaying = true;

        if (Notify_Slot)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Notify][5] ProcessNext - calling Notify_Slot->PlayNotify"));
            Notify_Slot->PlayNotify(NextData);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Notify][5] FAILED - Notify_Slot is null"));
        }
    }
    else
    {
        bIsPlaying = false;
    }
}

void UPTNotifyManagerWidget::HandleCurrentFinished()
{
    ProcessNext();
}
