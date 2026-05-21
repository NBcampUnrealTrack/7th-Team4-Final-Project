
#pragma once

#include "CoreMinimal.h"
#include "PTGameTypes.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Waiting,
    InProgress,
    GameClear,  // 보스 클리어
    GameOver    // 보스 전멸
};
