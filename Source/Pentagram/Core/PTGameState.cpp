#include "PTGameState.h"
#include "Net/UnrealNetwork.h"

void APTGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTGameState, CurrentPhase);
    DOREPLIFETIME(APTGameState, ElapsedTime);
}

void APTGameState::OnGamePhaseChanged()
{
}
