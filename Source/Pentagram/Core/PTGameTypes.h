#pragma once

#include "CoreMinimal.h"
#include "PTGameTypes.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Waiting,
    Playing,
    BossFight,
    GameClear,
    GameOver
};
