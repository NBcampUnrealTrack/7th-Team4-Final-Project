// Fill out your copyright notice in the Description page of Project Settings.


#include "PTEconomySubsystem.h"

#include "Character/Player/PTBasePlayerState.h"

void UPTEconomySubsystem::AddGold(APTBasePlayerState* PlayerState, int32 Amount)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority() || Amount <= 0)
    {
        return;
    }

    SetGold(PlayerState, PlayerState->CurrentGold + Amount);
}

bool UPTEconomySubsystem::SpendGold(APTBasePlayerState* PlayerState, int32 Amount)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return false;
    }

    if (!CanAfford(PlayerState, Amount))
    {
        return false;
    }

    SetGold(PlayerState, PlayerState->CurrentGold - Amount);
    return true;
}

void UPTEconomySubsystem::SetGold(APTBasePlayerState* PlayerState, int32 Amount)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return;
    }

    const int32 NewGold = FMath::Max(Amount, 0);
    if (PlayerState->CurrentGold == NewGold)
    {
        return;
    }

    PlayerState->CurrentGold = NewGold;
    OnGoldChanged.Broadcast(PlayerState, PlayerState->CurrentGold);
    PlayerState->OnGoldChanged.Broadcast(PlayerState->CurrentGold);
}

int32 UPTEconomySubsystem::GetGold(const APTBasePlayerState* PlayerState) const
{
    if (PlayerState == nullptr)
    {
        return 0;
    }

    return PlayerState->CurrentGold;
}

bool UPTEconomySubsystem::CanAfford(const APTBasePlayerState* PlayerState, int32 Amount) const
{
    if (PlayerState == nullptr || Amount <= 0)
    {
        return false;
    }

    return PlayerState->CurrentGold >= Amount;
}
