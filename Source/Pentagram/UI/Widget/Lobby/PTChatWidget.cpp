#include "PTChatWidget.h"
#include "Core/PTGameState.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

void UPTChatWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ChatInputBox)
    {
        ChatInputBox->OnTextCommitted.RemoveAll(this);
        ChatInputBox->OnTextCommitted.AddDynamic(this, &UPTChatWidget::OnChatInputCommitted);
    }

    if (APTGameState* PTGameState = GetWorld() ? GetWorld()->GetGameState<APTGameState>() : nullptr)
    {
        // 기존 로그 먼저 표시 (늦게 열려도 이전 대화 확인 가능)
        for (const FPTChatLogEntry& Entry : PTGameState->ChatLog)
        {
            AppendChatRow(Entry.SenderName, Entry.Message);
        }

        PTGameState->OnChatMessageReceived.RemoveDynamic(this, &UPTChatWidget::HandleChatMessageReceived);
        PTGameState->OnChatMessageReceived.AddDynamic(this, &UPTChatWidget::HandleChatMessageReceived);
    }
}

void UPTChatWidget::NativeDestruct()
{
    if (APTGameState* PTGameState = GetWorld() ? GetWorld()->GetGameState<APTGameState>() : nullptr)
    {
        PTGameState->OnChatMessageReceived.RemoveDynamic(this, &UPTChatWidget::HandleChatMessageReceived);
    }
    Super::NativeDestruct();
}

void UPTChatWidget::HandleChatMessageReceived(FString SenderName, FString Message)
{
    AppendChatRow(SenderName, Message);
}

void UPTChatWidget::AppendChatRow(const FString& SenderName, const FString& Message)
{
    if (!ChatScrollBox)
    {
        return;
    }

    UHorizontalBox* ChatRow = NewObject<UHorizontalBox>(this);

    UTextBlock* NicknameText = NewObject<UTextBlock>(this);
    NicknameText->SetText(FText::FromString(FString::Printf(TEXT("[%s]: "), *SenderName)));
    NicknameText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.3f)));

    UTextBlock* MessageText = NewObject<UTextBlock>(this);
    MessageText->SetText(FText::FromString(Message));
    MessageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    MessageText->SetAutoWrapText(true);

    ChatRow->AddChildToHorizontalBox(NicknameText);
    UHorizontalBoxSlot* MsgSlot = ChatRow->AddChildToHorizontalBox(MessageText);
    if (MsgSlot)
    {
        MsgSlot->SetSize(ESlateSizeRule::Fill);
    }

    ChatScrollBox->AddChild(ChatRow);
    ChatScrollBox->ScrollToEnd();
}

void UPTChatWidget::OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    if (CommitType != ETextCommit::OnEnter)
    {
        return;
    }

    const FString Trimmed = Text.ToString().TrimStartAndEnd();
    if (Trimmed.IsEmpty())
    {
        return;
    }

    if (APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PC->Server_SendChatMessage(Trimmed);
    }

    if (ChatInputBox)
    {
        ChatInputBox->SetText(FText::GetEmpty());
        ChatInputBox->SetKeyboardFocus();
    }
}
