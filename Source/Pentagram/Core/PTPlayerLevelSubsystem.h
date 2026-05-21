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

    void AddExp(int32 Amount);      // 경험치 추가
    void LevelUp();                  // 레벨업 처리
    int32 GetLevel();                // 현재 레벨 조회
    int32 GetExp();                  // 현재 경험치 조회
    void ApplyDeathPenalty();       // 사망 패널티

};
