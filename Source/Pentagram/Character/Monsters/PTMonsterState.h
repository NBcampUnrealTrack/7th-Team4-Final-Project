#pragma once

#include "CoreMinimal.h"
#include "PTMonsterState.generated.h"

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
    Idle    UMETA(DisplayName = "Idle"),
    Patrol  UMETA(DisplayName = "Patrol"),
    Chase   UMETA(DisplayName = "Chase"),
    Attack  UMETA(DisplayName = "Attack"),
    Dead    UMETA(DisplayName = "Dead")
};
