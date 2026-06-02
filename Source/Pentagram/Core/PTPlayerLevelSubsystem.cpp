// Fill out your copyright notice in the Description page of Project Settings.


#include "PTPlayerLevelSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"

void UPTPlayerLevelSubsystem::AddExp(APTBasePlayerState* PlayerState, int32 ExpAmount)
{
    if (PlayerState == nullptr || ExpAmount <= 0)
    {
        return;
    }

    if (PlayerState->RequiredExp <= 0)
    {
        PlayerState->RequiredExp = CalculateRequiredExp(PlayerState->PlayerLevel);
    }

    PlayerState->CurrentExp += ExpAmount;

    while (PlayerState->CurrentExp >= PlayerState->RequiredExp)
    {
        PlayerState->CurrentExp -= PlayerState->RequiredExp;
        LevelUp(PlayerState);
    }
}

void UPTPlayerLevelSubsystem::LevelUp(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    ++PlayerState->PlayerLevel;
    PlayerState->RequiredExp = CalculateRequiredExp(PlayerState->PlayerLevel);
}

int32 UPTPlayerLevelSubsystem::GetLevel(const APTBasePlayerState* PlayerState) const
{
    if (PlayerState == nullptr)
    {
        return 0;
    }

    return PlayerState->PlayerLevel;
}

int32 UPTPlayerLevelSubsystem::GetExp(const APTBasePlayerState* PlayerState) const
{
    if (PlayerState == nullptr)
    {
        return 0;
    }

    return PlayerState->CurrentExp;
}

int32 UPTPlayerLevelSubsystem::GetRequiredExp(const APTBasePlayerState* PlayerState) const
{
    if (PlayerState == nullptr)
    {
        return 0;
    }

    return PlayerState->RequiredExp;
}

void UPTPlayerLevelSubsystem::ApplyDeathPenalty(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    PlayerState->CurrentExp = FMath::Max(PlayerState->CurrentExp - DeathPenaltyExp, 0);
}

int32 UPTPlayerLevelSubsystem::CalculateRequiredExp(int32 PlayerLevel) const
{
    return FMath::Max(PlayerLevel, 1) * 100;
}
