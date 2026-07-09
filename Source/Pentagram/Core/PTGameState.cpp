#include "PTGameState.h"

#include "Subsystems/PTItemSubsystem.h"
#include "Subsystems/PTQuestSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/PTBasePlayerState.h"    // 로비 추가

void APTGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTGameState, CurrentPhase);
    DOREPLIFETIME(APTGameState, QuestDataTable);
    DOREPLIFETIME(APTGameState, ItemDataTable);
    DOREPLIFETIME(APTGameState, ChatLog); // 로비 채팅 추가
}

void APTGameState::SetCurrentPhase(EGamePhase NewPhase)
{
    if (!HasAuthority())
    {
        return;
    }
    if (CurrentPhase == NewPhase)
    {
        return;
    }

    CurrentPhase = NewPhase;

    OnGamePhaseChanged();
}

void APTGameState::SetQuestDataTable(UDataTable* InQuestDataTable)
{
    if (!HasAuthority())
    {
        return;
    }

    QuestDataTable = InQuestDataTable;
    ApplyQuestDataTable();
    ForceNetUpdate();
}

void APTGameState::SetItemDataTable(UDataTable* InItemDataTable)
{
    if (!HasAuthority())
    {
        return;
    }

    ItemDataTable = InItemDataTable;
    ApplyItemDataTable();
    ForceNetUpdate();
}

void APTGameState::OnRep_CurrentPhase()
{
    OnGamePhaseChanged();
}

void APTGameState::OnRep_QuestDataTable()
{
    ApplyQuestDataTable();
}

void APTGameState::OnRep_ItemDataTable()
{
    ApplyItemDataTable();
}

void APTGameState::OnGamePhaseChanged()
{
    OnGamePhaseChangedEvent.Broadcast(CurrentPhase);
}

// 로비 추가
void APTGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);

    NotifyLobbyUpdated();
}

// 로비 추가
void APTGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);

    NotifyLobbyUpdated();
}

// 로비 추가
void APTGameState::NotifyLobbyUpdated()
{
    OnLobbyUpdated.Broadcast();
}

// 로비 추가
int32 APTGameState::GetReadyCount() const
{
    int32 ReadyCount = 0;

    for (APlayerState* PlayerState : PlayerArray)
    {
        if (APTBasePlayerState* PTPlayerState = Cast<APTBasePlayerState>(PlayerState))
        {
            if (PTPlayerState->IsReady())
            {
                ++ReadyCount;
            }
        }
    }

    return ReadyCount;
}
void APTGameState::ApplyQuestDataTable() const
{
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->SetQuestDataTable(QuestDataTable);
    }
}

void APTGameState::ApplyItemDataTable() const
{
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTItemSubsystem* ItemSubsystem = GameInstance->GetSubsystem<UPTItemSubsystem>();
    if (ItemSubsystem != nullptr)
    {
        ItemSubsystem->SetItemDataTable(ItemDataTable);
    }
}

// 로비 채팅 추가
void APTGameState::Server_AddChatMessage(const FString& SenderName, const FString& Message)
{
    if (!HasAuthority())
    {
        return;
    }

    FPTChatLogEntry NewEntry;
    NewEntry.SenderName = SenderName;
    NewEntry.Message = Message;
    ChatLog.Add(NewEntry);

    if (ChatLog.Num() > MaxChatLogSize)
    {
        ChatLog.RemoveAt(0, ChatLog.Num() - MaxChatLogSize);
    }

    // ChatLog 배열은 늦게 들어온 플레이어의 과거 로그 조회용으로만 리플리케이트.
    // 실시간 알림은 Multicast RPC로 처리 -> 서버(리슨서버 호스트 포함) + 모든 클라이언트가 동시에 받음
    Multicast_ChatMessage(SenderName, Message);
}

// 로비 채팅 추가
void APTGameState::Multicast_ChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
    OnChatMessageReceived.Broadcast(SenderName, Message);
}
