#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTGameTypes.h"
#include "PTGameMode.generated.h"

class APTBasePlayerState;
class UDataTable;

UCLASS()
class PENTAGRAM_API APTGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APTGameMode();

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void RestartPlayerAtTransform(AController* PlayerController, const FTransform& SpawnTransform) override;

    void SetGamePhase(EGamePhase NewPhase);
    void StartGame();
    void EndGame();
    void OnBossFightStarted();
    void OnBossDefeated();
    void OnAllPlayersDead();
    void RespawnPlayer(AController* PlayerController);
    void RespawnPlayer(AController* PlayerController, const FVector& RespawnLoc, bool bHasCheckpoint);
    void DistributeExp(int32 ExpAmount);
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
    float RespawnDelaySeconds = 3.f;
};
