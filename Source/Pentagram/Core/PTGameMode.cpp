// Fill out your copyright notice in the Description page of Project Settings.


#include "PTGameMode.h"

#include "Character/Player/PTBasePlayerState.h"
#include "PTGameState.h"
#include "PTPlayerLevelSubsystem.h"
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
        RestartPlayerWithInitializedState(PlayerController);
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

            RestartPlayerWithInitializedState(StoredPlayerController);
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

    PlayerLevelSubsystem->AddExp(ExpAmount);
}

void APTGameMode::InitializePlayerState(APTBasePlayerState* PlayerState) const
{
    if (PlayerState == nullptr)
    {
        return;
    }

    PlayerState->MaxHP = DefaultMaxHP;
    PlayerState->CurrentHP = DefaultMaxHP;
    PlayerState->MaxMP = DefaultMaxMP;
    PlayerState->CurrentMP = DefaultMaxMP;
}

void APTGameMode::RestartPlayerWithInitializedState(APlayerController* PlayerController)
{
    if (PlayerController == nullptr)
    {
        return;
    }

    InitializePlayerState(PlayerController->GetPlayerState<APTBasePlayerState>());
    RestartPlayer(PlayerController);
}
