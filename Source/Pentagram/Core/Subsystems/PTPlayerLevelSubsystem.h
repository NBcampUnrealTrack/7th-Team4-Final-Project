#pragma once

#include "CoreMinimal.h"
#include "Core/PTLevelDataRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTPlayerLevelSubsystem.generated.h"

class APTBasePlayerState;
class UDataTable;

DECLARE_MULTICAST_DELEGATE_TwoParams(FPTNativeOnExpChanged, APTBasePlayerState*, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPTNativeOnLevelUp, APTBasePlayerState*, int32);

UCLASS()
class PENTAGRAM_API UPTPlayerLevelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void SetLevelDataTable(UDataTable* InLevelDataTable);
    void RebuildLevelDataMap();

    void AddExp(APTBasePlayerState* PlayerState, int32 Amount);
    int32 GetLevel(const APTBasePlayerState* PlayerState) const;
    int32 GetExp(const APTBasePlayerState* PlayerState) const;
    int32 GetRequiredExp(const APTBasePlayerState* PlayerState) const;
    void ApplyDeathPenalty(APTBasePlayerState* PlayerState);
    void SetProgress(APTBasePlayerState* PlayerState, int32 NewLevel, int32 NewExp);

private:
    void LevelUp(APTBasePlayerState* PlayerState);
    int32 CalculateRequiredExp(int32 PlayerLevel) const;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Level")
    TObjectPtr<UDataTable> LevelDataTable;

    TMap<int32, int32> RequiredExpByLevel;

    int32 DeathPenaltyExp = 0;

public:
    FPTNativeOnExpChanged OnExpChanged;
    FPTNativeOnLevelUp OnLevelUp;
};
