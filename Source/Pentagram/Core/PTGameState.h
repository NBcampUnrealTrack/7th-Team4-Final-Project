// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTGameTypes.h"
#include "GameFramework/GameStateBase.h"
#include "PTGameState.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API APTGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated)
    EGamePhase CurrentPhase;    // 게임 상태 (Waiting, InProgress, GameOver)

    UPROPERTY(Replicated)
    int32 ElapsedTime;          // 게임 진행 시간

    void OnGamePhaseChanged();  // 상태 변경 시 UI 이벤트 브로드캐스트
};
