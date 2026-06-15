#include "PTGameState.h"
#include "Net/UnrealNetwork.h"

void APTGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTGameState, CurrentPhase);
    DOREPLIFETIME(APTGameState, ElapsedTime);
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

