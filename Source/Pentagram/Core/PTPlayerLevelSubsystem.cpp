// Fill out your copyright notice in the Description page of Project Settings.


#include "PTPlayerLevelSubsystem.h"

void UPTPlayerLevelSubsystem::AddExp(int32 ExpAmount)
{
    if (ExpAmount <= 0)
    {
        return;
    }

    CurrentExp += ExpAmount;

    while (CurrentExp >= ExpToNextLevel)
    {
        CurrentExp -= ExpToNextLevel;
        LevelUp();
    }
}

void UPTPlayerLevelSubsystem::LevelUp()
{
    ++CurrentLevel;
}

int32 UPTPlayerLevelSubsystem::GetLevel() const
{
    return CurrentLevel;
}

int32 UPTPlayerLevelSubsystem::GetExp() const
{
    return CurrentExp;
}

void UPTPlayerLevelSubsystem::ApplyDeathPenalty()
{
    CurrentExp = FMath::Max(CurrentExp - DeathPenaltyExp, 0);
}
