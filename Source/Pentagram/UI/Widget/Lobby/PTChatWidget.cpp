#include "PTChatWidget.h"
#include "Core/PTGameState.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UPTChatWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ChatInputBox)
    {
        ChatInputBox->OnTextCommitted.RemoveAll(this);
        ChatInputBox->OnTextCommitted.AddDynamic(this, &UPTChatWidget::OnChatInputCommitted);
    }

    BindGameState();
}

void UPTChatWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BindRetryTimer);
    }

    if (BoundGameState)
    {
        BoundGameState->OnChatMessageReceived.RemoveDynamic(this, &UPTChatWidget::HandleChatMessageReceived);
        BoundGameState = nullptr;
    }

    Super::NativeDestruct();
}

void UPTChatWidget::BindGameState()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    APTGameState* PTGameState = World->GetGameState<APTGameState>();
    if (PTGameState == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Chat] BindGameState 재시도 (GameState 아직 없음)"));
        // 클라이언트는 접속 초기에 GameState가 아직 리플리케이트되지 않았을 수 있음 -> 재시도
        World->GetTimerManager().SetTimer(
            BindRetryTimer, this, &UPTChatWidget::BindGameState, 0.2f, false);
        return;
    }

    if (BoundGameState == PTGameState)
    {
        return; // 이미 바인딩됨
    }

    UE_LOG(LogTemp, Warning, TEXT("[Chat] BindGameState 성공. 기존 로그 %d개, IsServer=%d"),
        PTGameState->ChatLog.Num(), World->GetNetMode() != NM_Client);

    // 기존 로그 먼저 표시 (늦게 열려도, 재시도로 늦게 바인딩되어도 이전 대화 확인 가능)
    for (const FPTChatLogEntry& Entry : PTGameState->ChatLog)
    {
        AppendChatRow(Entry.SenderName, Entry.Message);
    }

    PTGameState->OnChatMessageReceived.RemoveDynamic(this, &UPTChatWidget::HandleChatMessageReceived);
    PTGameState->OnChatMessageReceived.AddDynamic(this, &UPTChatWidget::HandleChatMessageReceived);

    BoundGameState = PTGameState;
}

void UPTChatWidget::HandleChatMessageReceived(FString SenderName, FString Message)
{
    UE_LOG(LogTemp, Warning, TEXT("[Chat] HandleChatMessageReceived: %s: %s"), *SenderName, *Message);
    AppendChatRow(SenderName, Message);
}

void UPTChatWidget::AppendChatRow(const FString& SenderName, const FString& Message)
{
    if (!ChatScrollBox)
    {
        return;
    }

    // 폭 넘치면 자동으로 다음 줄로 내려감 (HorizontalBox는 그냥 잘려버려서 WrapBox로 교체)
    UWrapBox* ChatRow = NewObject<UWrapBox>(this);
    ChatRow->SetInnerSlotPadding(FVector2D(4.f, 2.f));

    FSlateFontInfo SmallFont = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14); // 폰트 크기 축소

    UTextBlock* NicknameText = NewObject<UTextBlock>(this);
    NicknameText->SetText(FText::FromString(FString::Printf(TEXT("[%s]: "), *SenderName)));
    NicknameText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.3f)));
    NicknameText->SetFont(SmallFont);
    NicknameText->SetAutoWrapText(true); // 닉네임 자체가 길 때도 잘리지 않고 줄바꿈

    UTextBlock* MessageText = NewObject<UTextBlock>(this);
    MessageText->SetText(FText::FromString(Message));
    MessageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    MessageText->SetFont(SmallFont);
    MessageText->SetAutoWrapText(true);

    ChatRow->AddChildToWrapBox(NicknameText);
    ChatRow->AddChildToWrapBox(MessageText);

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
