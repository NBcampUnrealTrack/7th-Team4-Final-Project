// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTGameMode.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API APTGameMode : public AGameModeBase
{
    GENERATED_BODY()


    void StartGame();                           // 게임 시작
    void EndGame();                             // 게임 종료
    void RespawnPlayer(APlayerController* PC);  // 리스폰
    void DistributeExp(int32 Amount);           // 경험치 분배
    //void DistributeDrop(Data Drop);    // 드롭 분배
};
