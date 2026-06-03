#include "Character/Player/PTBasePlayerState.h"
#include "UI/Data/PTDelegates.h"
#include "Net/UnrealNetwork.h"


void APTBasePlayerState::OnRep_CurrentHP()
{
    OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void APTBasePlayerState::OnRep_CurrentMP()
{
    OnManaChanged.Broadcast(CurrentMP, MaxMP);
}

void APTBasePlayerState::OnRep_RequiredExp()
{
    OnExpChanged.Broadcast(CurrentExp, RequiredExp);
}

void APTBasePlayerState::OnRep_PlayerLevel()
{
    OnLevelChanged.Broadcast(PlayerLevel);
}

void APTBasePlayerState::OnRep_CurrentGold()
{
    OnGoldChanged.Broadcast(CurrentGold);
}

void APTBasePlayerState::OnRep_CurrentExp()
{
    OnExpChanged.Broadcast(CurrentExp, RequiredExp);
}

void APTBasePlayerState::BroadcastAllStats()
{
    OnHealthChanged.Broadcast(CurrentHP, MaxHP);
    OnManaChanged.Broadcast(CurrentMP, MaxMP);
    OnLevelChanged.Broadcast(PlayerLevel);
    OnExpChanged.Broadcast(CurrentExp, RequiredExp);
    OnGoldChanged.Broadcast(CurrentGold);
}

void APTBasePlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTBasePlayerState, CurrentHP);
    DOREPLIFETIME(APTBasePlayerState, MaxHP);
    DOREPLIFETIME(APTBasePlayerState, CurrentMP);
    DOREPLIFETIME(APTBasePlayerState, MaxMP);
    DOREPLIFETIME(APTBasePlayerState, CurrentGold);
    DOREPLIFETIME(APTBasePlayerState, CurrentExp);
    DOREPLIFETIME(APTBasePlayerState, PlayerLevel);
    DOREPLIFETIME(APTBasePlayerState, RequiredExp);
}
