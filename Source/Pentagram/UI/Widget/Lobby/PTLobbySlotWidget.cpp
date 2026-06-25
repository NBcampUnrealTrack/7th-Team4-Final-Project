#include "PTLobbySlotWidget.h"
#include "Components/TextBlock.h"

void UPTLobbySlotWidget::SetSlot(const FString& InName, int32 InLevel, bool bInReady)
{
    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(InName));
    }
    if (PlayerLevelText)
    {
        PlayerLevelText->SetText(FText::AsNumber(InLevel));
    }
    if (ReadyStateText)
    {
        ReadyStateText->SetText(FText::FromString(bInReady ? TEXT("준비") : TEXT("대기")));
    }
}

void UPTLobbySlotWidget::SetEmpty()
{
    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(TEXT("빈 슬롯")));
    }
    if (PlayerLevelText)
    {
        PlayerLevelText->SetText(FText::GetEmpty());
    }
    if (ReadyStateText)
    {
        ReadyStateText->SetText(FText::GetEmpty());
    }
}
