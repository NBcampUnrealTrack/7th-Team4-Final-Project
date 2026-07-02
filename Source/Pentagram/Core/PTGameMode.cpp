// PTGameMode.cpp

#include "PTGameMode.h"

#include "Character/Player/PTBasePlayerState.h"
#include "PTGameState.h"
#include "Subsystems/PTPlayerLevelSubsystem.h"
#include "Subsystems/PTQuestSubsystem.h"
#include "Subsystems/PTItemSubsystem.h"
#include "Subsystems/PTLoadingSubsystem.h"
#include "Subsystems/PTSaveSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"


APTGameMode::APTGameMode()
{
    bUseSeamlessTravel = true;
    GameStateClass = APTGameState::StaticClass();
    PlayerStateClass = APTBasePlayerState::StaticClass();
}

void APTGameMode::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->SetQuestDataTable(QuestDataTable);
    }

    APTGameState* PTGameState = GetGameState<APTGameState>();
    if (PTGameState != nullptr)
    {
        PTGameState->SetQuestDataTable(QuestDataTable);
    }

    UPTPlayerLevelSubsystem* PlayerLevelSubsystem = GameInstance->GetSubsystem<UPTPlayerLevelSubsystem>();
    if (PlayerLevelSubsystem != nullptr)
    {
        PlayerLevelSubsystem->SetLevelDataTable(LevelDataTable);
    }

    UPTItemSubsystem* ItemSubsystem = GameInstance->GetSubsystem<UPTItemSubsystem>();
    if (ItemSubsystem != nullptr)
    {
        ItemSubsystem->SetItemDataTable(ItemDataTable);
    }

    if (PTGameState != nullptr)
    {
        PTGameState->SetItemDataTable(ItemDataTable);
    }

    StartAutoSaveIfAvailable();
}

void APTGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = NewPlayer->GetPlayerState<APTBasePlayerState>();
    InitializePlayerState(PlayerState);

    UGameInstance* GameInstance = GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    if (SaveSubsystem != nullptr)
    {
        SaveSubsystem->LoadPlayer(PlayerState);
    }

    StartAutoSaveIfAvailable();
}

void APTGameMode::Logout(AController* Exiting)
{
    SavePlayerState(Exiting);

    Super::Logout(Exiting);

    NotifyReadyChanged();
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

    SavePlayerState(PlayerController);

    if (RespawnDelaySeconds <= 0.f)
    {
        RestartPlayer(PlayerController);
        return;
    }

    TWeakObjectPtr<AController> WeakPlayerController(PlayerController);
    FTimerHandle RespawnTimerHandle;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, [this, WeakPlayerController]()
        {
            AController* ValidPlayerController = WeakPlayerController.Get();
            if (ValidPlayerController == nullptr)
            {
                return;
            }

            RestartPlayer(ValidPlayerController);
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

    SavePlayerState(PlayerController);

    FVector RespawnLocation = RespawnLoc;

    const FTransform RespawnTransform(FRotator::ZeroRotator, RespawnLocation);
    if (RespawnDelaySeconds <= 0.f)
    {
        RestartPlayerAtTransform(PlayerController, RespawnTransform);
        return;
    }

    TWeakObjectPtr<AController> WeakPlayerController(PlayerController);
    FTimerHandle RespawnTimerHandle;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, [this, WeakPlayerController, RespawnTransform]()
        {
            AController* ValidPlayerController = WeakPlayerController.Get();
            if (ValidPlayerController == nullptr)
            {
                return;
            }

            RestartPlayerAtTransform(ValidPlayerController, RespawnTransform);
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

// 로비 추가
void APTGameMode::NotifyReadyChanged()
{
    if (!HasAuthority() || bIsTraveling)    // 서버만 / 이동중 차단
    {
        return;
    }

    if (AreAllPlayersReady())
    {
        TravelToGame();
    }
}

// 로비 추가
bool APTGameMode::AreAllPlayersReady() const
{
    APTGameState* GS = GetGameState<APTGameState>();
    if (GS == nullptr)
    {
        return false;
    }

    int32 ValidCount = 0;
    for (APlayerState* PS : GS->PlayerArray)
    {
        APTBasePlayerState* PTPS = Cast<APTBasePlayerState>(PS);
        if (PTPS == nullptr || PTPS->IsInactive() || PTPS->IsOnlyASpectator())
        {
            continue;   // 집계 제외
        }

        if (!PTPS->IsReady())
        {
            return false;   // 미준비 차단
        }

        ++ValidCount;
    }

    return ValidCount >= MinPlayersToStart;   // 전원준비+인원
}

void APTGameMode::RequestTravelToGame()
{
    TravelToGame();
}

void APTGameMode::TravelToGame()
{
    if (bIsTraveling)    // 중복 차단
    {
        return;
    }

    APTGameState* GS = GetGameState<APTGameState>();
    if (GS == nullptr || GS->GetCurrentPhase() != EGamePhase::Waiting)
    {
        return;
    }

    if (GameMapPath.IsEmpty())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    bIsTraveling = true;
    SetGamePhase(EGamePhase::Loading);

    UE_LOG(LogTemp, Log, TEXT("[Loading] TravelToGame started. TargetMap=%s"), *GameMapPath);

    UPTLoadingSubsystem* LoadingSubsystem =
        GetGameInstance() != nullptr ? GetGameInstance()->GetSubsystem<UPTLoadingSubsystem>() : nullptr;
    if (LoadingSubsystem == nullptr)
    {
        HandleTravelPreloadComplete();
        return;
    }

    TArray<UDataTable*> PreloadDataTables;
    if (bPreloadItemDataTableForTravel && ItemDataTable != nullptr)
    {
        PreloadDataTables.Add(ItemDataTable);
    }

    for (UDataTable* PreloadDataTable : TravelPreloadDataTables)
    {
        if (PreloadDataTable != nullptr)
        {
            PreloadDataTables.AddUnique(PreloadDataTable);
        }
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Loading] Preload request. Assets=%d Classes=%d DataTables=%d"),
        TravelPreloadAssets.Num(),
        TravelPreloadClasses.Num(),
        PreloadDataTables.Num());

    LoadingSubsystem->PreloadForTravel(
        TravelPreloadAssets,
        TravelPreloadClasses,
        PreloadDataTables,
        FSimpleDelegate::CreateUObject(this, &APTGameMode::HandleTravelPreloadComplete));
}

void APTGameMode::HandleTravelPreloadComplete()
{
    UWorld* World = GetWorld();
    if (World == nullptr || GameMapPath.IsEmpty())
    {
        bIsTraveling = false;
        return;
    }

    World->ServerTravel(GameMapPath);
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

    UGameInstance* GameInstance = GetGameInstance();
    UPTPlayerLevelSubsystem* PlayerLevelSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTPlayerLevelSubsystem>() : nullptr;
    if (PlayerLevelSubsystem != nullptr)
    {
        PlayerLevelSubsystem->SetProgress(PlayerState, PlayerState->PlayerLevel, PlayerState->CurrentExp);
        return;
    }

    PlayerState->RequiredExp = FMath::Max(PlayerState->RequiredExp, 100);
}

void APTGameMode::SavePlayerState(AController* PlayerController) const
{
    if (PlayerController == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = PlayerController->GetPlayerState<APTBasePlayerState>();
    if (PlayerState == nullptr)
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    if (SaveSubsystem == nullptr)
    {
        return;
    }

    SaveSubsystem->SavePlayer(PlayerState);
}

void APTGameMode::StartAutoSaveIfAvailable() const
{
    UGameInstance* GameInstance = GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    if (SaveSubsystem != nullptr)
    {
        SaveSubsystem->NotifyWorldReadyForAutoSave();
    }
}

void APTGameMode::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();

    UGameInstance* GameInstance = GetGameInstance();
    UPTLoadingSubsystem* LoadingSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTLoadingSubsystem>() : nullptr;
    if (LoadingSubsystem != nullptr)
    {
        LoadingSubsystem->FinishLoading();
    }

    StartAutoSaveIfAvailable();
}

void APTGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);
    StartAutoSaveIfAvailable();
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
        FinalSpawnTransform.SetLocation(RespawnLocation);
    }

    Super::RestartPlayerAtTransform(PlayerController, FinalSpawnTransform);
}
