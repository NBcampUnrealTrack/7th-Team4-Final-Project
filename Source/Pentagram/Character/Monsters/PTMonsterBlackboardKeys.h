#pragma once

#include "CoreMinimal.h"

namespace PTMonsterBlackboardKeys
{
    const FName TargetActor         = TEXT("TargetActor");
    const FName IsTargetDetected    = TEXT("IsTargetDetected");
    const FName IsInAttackRange      = TEXT("IsInAttackRange");
    const FName CanAttack           = TEXT("CanAttack");
    const FName IsEnraged           = TEXT("IsEnraged");    // 보스 전용
    const FName PatrolLocation      = TEXT("PatrolLocation");
    const FName SpawnLocation       = TEXT("SpawnLocation");
}
