// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTPlayerLevelSubsystem.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTPlayerLevelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void AddExp(int32 Amount);      // 경험치 추가
    int32 GetLevel() const;          // 현재 레벨 조회
    int32 GetExp() const;            // 현재 경험치 조회
    void ApplyDeathPenalty();       // 사망 패널티

private:
    void LevelUp();                  // 레벨업 처리

    int32 CurrentLevel = 1;
    int32 CurrentExp = 0;
    int32 ExpToNextLevel = 100;
    int32 DeathPenaltyExp = 0;
};
