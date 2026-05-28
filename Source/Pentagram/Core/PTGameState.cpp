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

    OnGamePhaseChanged(); // // OnRep은 서버에서 호출되지 않으므로 서버에서도 동일하게 이벤트가 발생하도록 직접 호출.
}

void APTGameState::OnRep_CurrentPhase()
{
    OnGamePhaseChanged();
}

void APTGameState::OnGamePhaseChanged()
{
   //OnGamePhaseChangedEvent.Broadcast(CurrentPhase);
}

