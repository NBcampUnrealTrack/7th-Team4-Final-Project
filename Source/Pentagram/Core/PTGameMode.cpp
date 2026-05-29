// Fill out your copyright notice in the Description page of Project Settings.


#include "PTGameMode.h"

#include "Character/Player/PTBasePlayerState.h"
#include "PTGameState.h"
#include "PTPlayerLevelSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

APTGameMode::APTGameMode()
{
    GameStateClass = APTGameState::StaticClass();
    PlayerStateClass = APTBasePlayerState::StaticClass();
}

void APTGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer == nullptr)
    {
        return;
    }

    InitializePlayerState(NewPlayer->GetPlayerState<APTBasePlayerState>());
}

void APTGameMode::StartGame()
{
    APTGameState* PTGameState = GetGameState<APTGameState>();
    if (PTGameState == nullptr)
    {
        return;
    }
    PTGameState->SetCurrentPhase(EGamePhase::InProgress);
}

void APTGameMode::EndGame()
{
    APTGameState* PTGameState = GetGameState<APTGameState>();
    if (PTGameState == nullptr)
    {
        return;
    }

    PTGameState->SetCurrentPhase(EGamePhase::GameOver);
}

void APTGameMode::RespawnPlayer(APlayerController* PlayerController)
{
    if (PlayerController == nullptr)
    {
        return;
    }

    if (RespawnDelaySeconds <= 0.f)
    {
        RestartPlayer(PlayerController);
        return;
    }

    TWeakObjectPtr<APlayerController> WeakPlayerController = PlayerController;
    FTimerDelegate RespawnDelegate = FTimerDelegate::CreateWeakLambda(this,
        [this, WeakPlayerController]()
        {
            APlayerController* StoredPlayerController = WeakPlayerController.Get();
            if (StoredPlayerController == nullptr)
            {
                return;
            }

            RestartPlayer(StoredPlayerController);
        });

    FTimerHandle RespawnTimerHandle;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnDelaySeconds, false);
}

void APTGameMode::DistributeExp(int32 ExpAmount)
{
    if (ExpAmount <= 0)
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTPlayerLevelSubsystem* PlayerLevelSubsystem = GameInstance->GetSubsystem<UPTPlayerLevelSubsystem>();
    if (PlayerLevelSubsystem == nullptr)
    {
        return;
    }

    APTGameState* PTGameState = GetGameState<APTGameState>();
    if (PTGameState == nullptr)
    {
        return;
    }

    for (APlayerState* PlayerState : PTGameState->PlayerArray)
    {
        APTBasePlayerState* PTPlayerState = Cast<APTBasePlayerState>(PlayerState);
        if (PTPlayerState == nullptr)
        {
            continue;
        }

        PlayerLevelSubsystem->AddExp(PTPlayerState, ExpAmount);
    }
}

AActor* APTGameMode::SpawnDropItem(TSubclassOf<AActor> DropItemClass, const FVector& DropLocation) const
{
    if (!HasAuthority() || DropItemClass == nullptr)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    return World->SpawnActor<AActor>(
        DropItemClass,
        DropLocation,
        FRotator::ZeroRotator,
        SpawnParameters);
}

AActor* APTGameMode::SpawnDropItemByChance(TSubclassOf<AActor> DropItemClass, const FVector& DropLocation, float DropRate) const
{
    const float ClampedDropRate = FMath::Clamp(DropRate, 0.f, 1.f);
    if (ClampedDropRate <= 0.f || FMath::FRand() > ClampedDropRate)
    {
        return nullptr;
    }

    return SpawnDropItem(DropItemClass, DropLocation);
}

void APTGameMode::InitializePlayerState(APTBasePlayerState* PlayerState) const
{
    if (PlayerState == nullptr)
    {
        return;
    }

    PlayerState->PlayerLevel = FMath::Max(PlayerState->PlayerLevel, 1);
    PlayerState->CurrentExp = FMath::Max(PlayerState->CurrentExp, 0);
    PlayerState->RequiredExp = FMath::Max(PlayerState->RequiredExp, 100);
    PlayerState->CurrentGold = FMath::Max(PlayerState->CurrentGold, 0);
}
