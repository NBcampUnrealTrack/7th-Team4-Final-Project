// PTGameMode.h 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTGameTypes.h"
#include "PTGameMode.generated.h"

class APTBasePlayerState;
class UDataTable;

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
    virtual void RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform) override;

    void SetGamePhase(EGamePhase NewPhase);     //게임 상태 설정
    void StartGame();       //스타트 게임
    void EndGame();         //앤드게임
    void OnBossFightStarted(); //보스전 시작
    void OnBossDefeated();  //보스 처치
    void OnAllPlayersDead(); //플레이어 전원 사망
    void RespawnPlayer(AController* NewPlayer); //리스폰 플레이어
    void DistributeExp(int32 ExpAmount);        //exp 분배
    AActor* SpawnDropItem(TSubclassOf<AActor> DropItemClass, const FVector& DropLocation) const;
    AActor* SpawnDropItemByChance(TSubclassOf<AActor> DropItemClass, const FVector& DropLocation, float DropRate) const;

protected:
    virtual void BeginPlay() override;

private:
    void InitializePlayerState(APTBasePlayerState* PlayerState) const;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "PT|Quest")
    TObjectPtr<UDataTable> QuestDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Level")
    TObjectPtr<UDataTable> LevelDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Respawn")
    float RespawnDelaySeconds = 3.f;        //리스폰 대기 시간

};
