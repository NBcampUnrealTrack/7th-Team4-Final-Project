#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTGameTypes.h"
#include "PTGameMode.generated.h"

class APlayerController;
class APTBasePlayerState;
class UDataTable;

UCLASS()
class PENTAGRAM_API APTGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APTGameMode();

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;    // 로비 추가
    virtual void RestartPlayerAtTransform(AController* PlayerController, const FTransform& SpawnTransform) override;
    virtual void PostSeamlessTravel() override;
    virtual void HandleSeamlessTravelPlayer(AController*& C) override;

    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override; // [레벨트리거] 태그 기반 스폰 위치 선택 

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

    // 준비 변경 시 호출 로비 추가
    void NotifyReadyChanged();
    bool AreAllPlayersReady() const;

    UFUNCTION(BlueprintCallable, Category = "PT|Lobby")
    void RequestTravelToGame();

protected:
    virtual void BeginPlay() override;
    virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
    virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;

private:
    void InitializePlayerState(APTBasePlayerState* PlayerState) const;
    void SavePlayerState(AController* PlayerController) const;
    void ApplyPendingPlayerCharacterData(AController* PlayerController) const;
    void StartAutoSaveIfAvailable() const;
    FString ResolveGameMapPath() const;

    void TravelToGame();    // 서버 트래블 로비 추가
    void HandleTravelPreloadComplete();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "PT|Quest")
    TObjectPtr<UDataTable> QuestDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Level")
    TObjectPtr<UDataTable> LevelDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Item")
    TObjectPtr<UDataTable> ItemDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Loading")
    TArray<TSoftObjectPtr<UObject>> TravelPreloadAssets;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Loading")
    TArray<TSoftClassPtr<UObject>> TravelPreloadClasses;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Loading")
    TArray<TObjectPtr<UDataTable>> TravelPreloadDataTables;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Loading")
    bool bPreloadItemDataTableForTravel = true;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Respawn")
    float RespawnDelaySeconds = 3.f;

    // 인게임 맵 경로 로비 추가
    UPROPERTY(EditDefaultsOnly, Category = "PT|Lobby")
    FString GameMapPath;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Lobby")
    FString FallbackGameMapPath = TEXT("/Game/Pentagram/Level/01_Abandoned_Mine/L_Abandoned_Mine_01_Quarry");

    // 시작 최소 인원 로비 추가
    UPROPERTY(EditDefaultsOnly, Category = "PT|Lobby")
    int32 MinPlayersToStart = 2;

private:
    // 로비 추가
    bool bIsTraveling = false;
};
