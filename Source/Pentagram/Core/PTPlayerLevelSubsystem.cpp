// Fill out your copyright notice in the Description page of Project Settings.


#include "PTPlayerLevelSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Engine/DataTable.h"

void UPTPlayerLevelSubsystem::SetLevelDataTable(UDataTable* InLevelDataTable)
{
    LevelDataTable = InLevelDataTable;
    RebuildLevelDataMap();
}

void UPTPlayerLevelSubsystem::RebuildLevelDataMap()
{
    RequiredExpByLevel.Empty();

    if (LevelDataTable == nullptr)
    {
        return;
    }

    TArray<FPTLevelDataRow*> LevelRows;
    LevelDataTable->GetAllRows<FPTLevelDataRow>(TEXT("Level Data Map"), LevelRows);

    for (const FPTLevelDataRow* LevelRow : LevelRows)
    {
        const int32 PlayerLevel = FMath::Max(LevelRow->PlayerLevel, 1);
        const int32 RequiredExp = FMath::Max(LevelRow->RequiredExp, 1);
        RequiredExpByLevel.Add(PlayerLevel, RequiredExp);
    }
}

void UPTPlayerLevelSubsystem::AddExp(APTBasePlayerState* PlayerState, int32 ExpAmount)
{
    if (PlayerState == nullptr || ExpAmount <= 0)
    {
        return;
    }

    PlayerState->RequiredExp = CalculateRequiredExp(PlayerState->PlayerLevel);

    PlayerState->CurrentExp += ExpAmount;

    while (PlayerState->CurrentExp >= PlayerState->RequiredExp)
    {
        PlayerState->CurrentExp -= PlayerState->RequiredExp;
        LevelUp(PlayerState);
    }

    OnExpChanged.Broadcast(PlayerState, PlayerState->CurrentExp);
}

void UPTPlayerLevelSubsystem::LevelUp(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    ++PlayerState->PlayerLevel;
    PlayerState->RequiredExp = CalculateRequiredExp(PlayerState->PlayerLevel);
    OnLevelUp.Broadcast(PlayerState, PlayerState->PlayerLevel);
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

    const int32 NewExp = FMath::Max(PlayerState->CurrentExp - DeathPenaltyExp, 0);
    if (PlayerState->CurrentExp == NewExp)
    {
        return;
    }

    PlayerState->CurrentExp = NewExp;
    OnExpChanged.Broadcast(PlayerState, PlayerState->CurrentExp);
}

void UPTPlayerLevelSubsystem::SetProgress(APTBasePlayerState* PlayerState, int32 NewLevel, int32 NewExp)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    PlayerState->PlayerLevel = FMath::Max(NewLevel, 1);
    PlayerState->CurrentExp = FMath::Max(NewExp, 0);
    PlayerState->RequiredExp = CalculateRequiredExp(PlayerState->PlayerLevel);

    OnExpChanged.Broadcast(PlayerState, PlayerState->CurrentExp);
}

int32 UPTPlayerLevelSubsystem::CalculateRequiredExp(int32 PlayerLevel) const
{
    const int32 SafePlayerLevel = FMath::Max(PlayerLevel, 1);
    const int32* RequiredExp = RequiredExpByLevel.Find(SafePlayerLevel);
    if (RequiredExp != nullptr)
    {
        return *RequiredExp;
    }

    return SafePlayerLevel * 100;
}
