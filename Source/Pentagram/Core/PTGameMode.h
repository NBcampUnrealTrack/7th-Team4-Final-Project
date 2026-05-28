// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTGameMode.generated.h"

class APTBasePlayerState;

/**
 *
 */
UCLASS()
class PENTAGRAM_API APTGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APTGameMode();      //생성자

    virtual void PostLogin(APlayerController* NewPlayer) override;      //플레이어 로그인
    void StartGame();       //스타트 게임
    void EndGame();         //앤드게임
    void RespawnPlayer(APlayerController* PlayerController);        //리스폰 플레이어
    void DistributeExp(int32 ExpAmount);        //exp 분배
    //void DistributeDrop(Data Drop);    // 드롭 분배

protected:
    UPROPERTY(EditDefaultsOnly, Category = "PT|GameMode|Respawn")
    float RespawnDelaySeconds = 3.f;        //리스폰 대기 시간

private:
    void InitializePlayerState(APTBasePlayerState* PlayerState) const;

};
