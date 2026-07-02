#pragma once

#include "CoreMinimal.h"

namespace PTMonsterBlackboardKeys
{   // 공통
    inline const FName TargetActor         = TEXT("TargetActor");
    inline const FName IsTargetDetected    = TEXT("IsTargetDetected");
    inline const FName IsInAttackRange     = TEXT("IsInAttackRange");
    inline const FName CanAttack           = TEXT("CanAttack");
    inline const FName SpawnLocation       = TEXT("SpawnLocation");
    inline const FName LastKnownLocation   = TEXT("LastKnownLocation");
    inline const FName IsRanged            = TEXT("IsRanged");
    // EQS
    inline const FName MoveLocation        = TEXT("MoveLocation");
    // 일반
    inline const FName PatrolLocation      = TEXT("PatrolLocation");
    inline const FName ShouldReturnToSpawn = TEXT("ShouldReturnToSpawn");
    // 보스
    inline const FName BossPhase           = TEXT("BossPhase");
    inline const FName PatternLocation     = TEXT("PatternLocation");
    inline const FName IsPatternActive     = TEXT("IsPatternActive");
    inline const FName OptimalRange        = TEXT("OptimalRange");
}
