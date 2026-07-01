#pragma once

#include "CoreMinimal.h"
#include "PTGameTypes.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Waiting,
    Loading,
    Playing,
    BossFight,
    GameClear,
    GameOver
};
