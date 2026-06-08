// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTLevelDataRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTPlayerLevelSubsystem.generated.h"

class APTBasePlayerState;
class UDataTable;

DECLARE_MULTICAST_DELEGATE_TwoParams(FPTNativeOnExpChanged, APTBasePlayerState*, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPTNativeOnLevelUp, APTBasePlayerState*, int32);

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTPlayerLevelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void SetLevelDataTable(UDataTable* InLevelDataTable);
    void RebuildLevelDataMap();

    void AddExp(APTBasePlayerState* PlayerState, int32 Amount);      // 경험치 추가
    int32 GetLevel(const APTBasePlayerState* PlayerState) const;          // 현재 레벨 조회
    int32 GetExp(const APTBasePlayerState* PlayerState) const;            // 현재 경험치 조회
    int32 GetRequiredExp(const APTBasePlayerState* PlayerState) const;     // 다음 레벨 필요 경험치 조회
    void ApplyDeathPenalty(APTBasePlayerState* PlayerState);       // 사망 패널티
    void SetProgress(APTBasePlayerState* PlayerState, int32 NewLevel, int32 NewExp);

    FPTNativeOnExpChanged OnExpChanged;
    FPTNativeOnLevelUp OnLevelUp;

private:
    void LevelUp(APTBasePlayerState* PlayerState);                  // 레벨업 처리
    int32 CalculateRequiredExp(int32 PlayerLevel) const;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Level")
    TObjectPtr<UDataTable> LevelDataTable;

    TMap<int32, int32> RequiredExpByLevel;

    int32 DeathPenaltyExp = 0;
};
