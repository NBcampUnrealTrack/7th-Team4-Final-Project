// Fill out your copyright notice in the Description page of Project Settings.


#include "PTGameMode.h"
#include "PTGameState.h"

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


}
