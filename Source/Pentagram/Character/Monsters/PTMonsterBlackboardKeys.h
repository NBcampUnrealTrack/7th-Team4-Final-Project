#pragma once

#include "CoreMinimal.h"

namespace PTMonsterBlackboardKeys
{
    inline const FName TargetActor         = TEXT("TargetActor");
    inline const FName IsTargetDetected    = TEXT("IsTargetDetected");
    inline const FName IsInAttackRange     = TEXT("IsInAttackRange");
    inline const FName CanAttack           = TEXT("CanAttack");
    inline const FName IsEnraged           = TEXT("IsEnraged");    // 보스 전용
    inline const FName PatrolLocation      = TEXT("PatrolLocation");
    inline const FName SpawnLocation       = TEXT("SpawnLocation");
}
