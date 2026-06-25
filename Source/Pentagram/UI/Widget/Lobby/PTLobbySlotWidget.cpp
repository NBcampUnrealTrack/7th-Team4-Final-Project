#include "PTLobbySlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UPTLobbySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SlotNumberText)
    {
        SlotNumberText->SetText(FText::AsNumber(SlotNumber));
    }
}

void UPTLobbySlotWidget::SetSlot(const FString& InName, int32 InLevel, bool bInReady)
{
    if (Img_Background && FilledBackground)
    {
        Img_Background->SetBrushFromTexture(FilledBackground);
    }

    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(InName));
        PlayerNameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    if (PlayerLevelText)
    {
        PlayerLevelText->SetText(FText::FromString(FString::Printf(TEXT("LV. %d"), InLevel)));
        PlayerLevelText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    if (ReadyStateText)
    {
        ReadyStateText->SetText(FText::FromString(bInReady ? TEXT("Ready") : TEXT("Waiting")));
        ReadyStateText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

void UPTLobbySlotWidget::SetEmpty()
{
    if (Img_Background && EmptyBackground)
    {
        Img_Background->SetBrushFromTexture(EmptyBackground);
    }

    if (PlayerNameText)
    {
        PlayerNameText->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (PlayerLevelText)
    {
        PlayerLevelText->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (ReadyStateText)
    {
        ReadyStateText->SetVisibility(ESlateVisibility::Collapsed);
    }
}
