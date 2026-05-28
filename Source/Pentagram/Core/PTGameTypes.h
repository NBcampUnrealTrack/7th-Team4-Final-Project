
#pragma once

#include "CoreMinimal.h"
#include "PTGameTypes.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Waiting,    //게임 진입 전
    InProgress, //게임 진행
    GameClear,  // 보스 클리어
    GameOver    // 보스 전멸
};
