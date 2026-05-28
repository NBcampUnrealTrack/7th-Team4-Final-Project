// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTPlayerLevelSubsystem.generated.h"

class APTBasePlayerState;

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTPlayerLevelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void AddExp(APTBasePlayerState* PlayerState, int32 Amount);      // 경험치 추가
    int32 GetLevel(const APTBasePlayerState* PlayerState) const;          // 현재 레벨 조회
    int32 GetExp(const APTBasePlayerState* PlayerState) const;            // 현재 경험치 조회
    int32 GetRequiredExp(const APTBasePlayerState* PlayerState) const;     // 다음 레벨 필요 경험치 조회
    void ApplyDeathPenalty(APTBasePlayerState* PlayerState);       // 사망 패널티

private:
    void LevelUp(APTBasePlayerState* PlayerState);                  // 레벨업 처리
    int32 CalculateRequiredExp(int32 PlayerLevel) const;

    int32 DeathPenaltyExp = 0;
};
