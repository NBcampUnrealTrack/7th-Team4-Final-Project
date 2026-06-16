#include "PTGameState.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/PTBasePlayerState.h"    // 로비 추가

void APTGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTGameState, CurrentPhase);
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

void APTGameState::OnRep_CurrentPhase()
{
    OnGamePhaseChanged();
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
