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
    if (PlayerState == nullptr || !PlayerState->HasAuthority() || ExpAmount <= 0)
    {
        return;
    }

    PlayerState->RequiredExp = CalculateRequiredExp(PlayerState->PlayerLevel);

    const int64 NewExp = static_cast<int64>(PlayerState->CurrentExp) + static_cast<int64>(ExpAmount);
    PlayerState->CurrentExp = static_cast<int32>(FMath::Clamp<int64>(NewExp, 0, MAX_int32));

    int32 LevelUpCount = 0;
    while (PlayerState->RequiredExp > 0 &&
        PlayerState->CurrentExp >= PlayerState->RequiredExp &&
        LevelUpCount < 1000)
    {
        PlayerState->CurrentExp -= PlayerState->RequiredExp;
        LevelUp(PlayerState);
        ++LevelUpCount;
    }

    OnExpChanged.Broadcast(PlayerState, PlayerState->CurrentExp);
    PlayerState->OnExpChanged.Broadcast(PlayerState->CurrentExp, PlayerState->RequiredExp);
}

void UPTPlayerLevelSubsystem::LevelUp(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return;
    }

    if (PlayerState->PlayerLevel >= MAX_int32)
    {
        return;
    }

    ++PlayerState->PlayerLevel;
    PlayerState->RequiredExp = CalculateRequiredExp(PlayerState->PlayerLevel);
    PlayerState->OnLevelChanged.Broadcast(PlayerState->PlayerLevel);
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
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
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
    PlayerState->OnExpChanged.Broadcast(PlayerState->CurrentExp, PlayerState->RequiredExp);
}

void UPTPlayerLevelSubsystem::SetProgress(APTBasePlayerState* PlayerState, int32 NewLevel, int32 NewExp)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return;
    }

    PlayerState->PlayerLevel = FMath::Max(NewLevel, 1);
    PlayerState->CurrentExp = FMath::Max(NewExp, 0);
    PlayerState->RequiredExp = CalculateRequiredExp(PlayerState->PlayerLevel);

    PlayerState->OnLevelChanged.Broadcast(PlayerState->PlayerLevel);
    PlayerState->OnExpChanged.Broadcast(PlayerState->CurrentExp, PlayerState->RequiredExp);
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

    const int64 FallbackRequiredExp = static_cast<int64>(SafePlayerLevel) * 100;
    return static_cast<int32>(FMath::Clamp<int64>(FallbackRequiredExp, 1, MAX_int32));
}
