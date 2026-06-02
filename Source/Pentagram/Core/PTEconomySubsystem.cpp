// Fill out your copyright notice in the Description page of Project Settings.


#include "PTEconomySubsystem.h"

#include "Character/Player/PTBasePlayerState.h"

void UPTEconomySubsystem::AddGold(APTBasePlayerState* PlayerState, int32 Amount)
{
    if (PlayerState == nullptr || Amount <= 0)
    {
        return;
    }

    PlayerState->CurrentGold += Amount;
}

bool UPTEconomySubsystem::SpendGold(APTBasePlayerState* PlayerState, int32 Amount)
{
    if (!CanAfford(PlayerState, Amount))
    {
        return false;
    }

    PlayerState->CurrentGold -= Amount;
    return true;
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
