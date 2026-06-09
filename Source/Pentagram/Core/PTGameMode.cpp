// PTGameMode.cpp

#include "PTGameMode.h"

#include "Character/Player/PTBasePlayerState.h"
#include "PTGameState.h"
#include "Subsystems/PTPlayerLevelSubsystem.h"
#include "Subsystems/PTQuestSubsystem.h"
#include "Subsystems/PTSaveSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"


APTGameMode::APTGameMode()
{
    GameStateClass = APTGameState::StaticClass();
    PlayerStateClass = APTBasePlayerState::StaticClass();
}

void APTGameMode::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GameInstance = GetGameInstance();
    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->SetQuestDataTable(QuestDataTable);
    }

    UPTPlayerLevelSubsystem* PlayerLevelSubsystem = GameInstance->GetSubsystem<UPTPlayerLevelSubsystem>();
    if (PlayerLevelSubsystem != nullptr)
    {
        PlayerLevelSubsystem->SetLevelDataTable(LevelDataTable);
    }
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

void APTGameMode::SetGamePhase(EGamePhase NewPhase)
{
    APTGameState* PTGameState = GetGameState<APTGameState>();
    if (PTGameState == nullptr)
    {
        return;
    }

    PTGameState->SetCurrentPhase(NewPhase);
}

void APTGameMode::StartGame()
{
    SetGamePhase(EGamePhase::Playing);
}

void APTGameMode::EndGame()
{
    SetGamePhase(EGamePhase::GameOver);
}

void APTGameMode::OnBossFightStarted()
{
    SetGamePhase(EGamePhase::BossFight);
}

void APTGameMode::OnBossDefeated()
{
    SetGamePhase(EGamePhase::GameClear);
}

void APTGameMode::OnAllPlayersDead()
{
    SetGamePhase(EGamePhase::GameOver);
}

void APTGameMode::RespawnPlayer(AController* PlayerController)
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

    FTimerHandle RespawnTimerHandle;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, [this, PlayerController]()
        {
            if (PlayerController == nullptr)
            {
                return;
            }

            RestartPlayer(PlayerController);
        }, RespawnDelaySeconds, false);
}

void APTGameMode::RespawnPlayer(AController* PlayerController, const FVector& RespawnLoc, bool bHasCheckpoint)
{
    if (PlayerController == nullptr)
    {
        return;
    }

    if (!bHasCheckpoint)
    {
        RespawnPlayer(PlayerController);
        return;
    }

    FVector RespawnLocation = RespawnLoc;
    RespawnLocation.Z += 150.f;

    const FTransform RespawnTransform(FRotator::ZeroRotator, RespawnLocation);
    if (RespawnDelaySeconds <= 0.f)
    {
        RestartPlayerAtTransform(PlayerController, RespawnTransform);
        return;
    }

    FTimerHandle RespawnTimerHandle;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, [this, PlayerController, RespawnTransform]()
        {
            if (PlayerController == nullptr)
            {
                return;
            }

            RestartPlayerAtTransform(PlayerController, RespawnTransform);
        }, RespawnDelaySeconds, false);
}

void APTGameMode::DistributeExp(int32 ExpAmount)
{
    if (ExpAmount <= 0)
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
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
        if (APTBasePlayerState* PTPlayerState = Cast<APTBasePlayerState>(PlayerState))
        {
            PlayerLevelSubsystem->AddExp(PTPlayerState, ExpAmount);
        }
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
    PlayerState->CurrentGold = FMath::Max(PlayerState->CurrentGold, 0);

    UPTPlayerLevelSubsystem* PlayerLevelSubsystem =
        GetGameInstance()->GetSubsystem<UPTPlayerLevelSubsystem>();
    if (PlayerLevelSubsystem != nullptr)
    {
        PlayerLevelSubsystem->SetProgress(PlayerState, PlayerState->PlayerLevel, PlayerState->CurrentExp);
        return;
    }

    PlayerState->RequiredExp = FMath::Max(PlayerState->RequiredExp, 100);
}

void APTGameMode::RestartPlayerAtTransform(AController* PlayerController, const FTransform& SpawnTransform)
{
    if (PlayerController == nullptr)
    {
        return;
    }

    FTransform FinalSpawnTransform = SpawnTransform;

    APTBasePlayerState* PlayerState = PlayerController->GetPlayerState<APTBasePlayerState>();
    if (PlayerState != nullptr && PlayerState->HasRespawnLocation())
    {
        FVector RespawnLocation = PlayerState->GetSavedRespawnLocation();
        RespawnLocation.Z += 150.f;
        FinalSpawnTransform.SetLocation(RespawnLocation);
    }

    Super::RestartPlayerAtTransform(PlayerController, FinalSpawnTransform);
}
