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

