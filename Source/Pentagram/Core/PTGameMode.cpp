// PTGameMode.cpp

#include "PTGameMode.h"

#include "GameFramework/PlayerController.h"
#include "Character/Player/PTBasePlayerState.h"
#include "PTGameState.h"
#include "Subsystems/PTPlayerLevelSubsystem.h"
#include "Subsystems/PTQuestSubsystem.h"
#include "Subsystems/PTItemSubsystem.h"
#include "Subsystems/PTLoadingSubsystem.h"
#include "Subsystems/PTSaveSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

// [레벨트리거] 태그 검색 및 옵션 추출을 위함
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const FString LegacyMissingGameMapPath = TEXT("/Game/Pentagram/Level/Field_1");
}

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

void APTGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
    Super::FinishRestartPlayer(NewPlayer, StartRotation);
    ApplyPendingPlayerCharacterData(NewPlayer);
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

void APTGameMode::NotifyReadyChanged()
{
    if (!HasAuthority() || bIsTraveling)
    {
        return;
    }

    if (AreAllPlayersReady())
    {
        TravelToGame();
    }
}

bool APTGameMode::AreAllPlayersReady() const
{
    APTGameState* GS = GetGameState<APTGameState>();
    if (GS == nullptr)
    {
        return false;
    }

    int32 ValidCount = 0;
    int32 ReadyCount = 0;

    for (APlayerState* PS : GS->PlayerArray)
    {
        APTBasePlayerState* PTPS = Cast<APTBasePlayerState>(PS);
        if (PTPS == nullptr || PTPS->IsInactive() || PTPS->IsOnlyASpectator())
        {
            continue;
        }

        ++ValidCount;

        if (PTPS->IsReady())
        {
            ++ReadyCount;
        }
    }

    if (ValidCount < FMath::Max(MinPlayersToStart, 1))
    {
        return false;
    }

    return ReadyCount == ValidCount;
}

void APTGameMode::RequestTravelToGame()
{
    TravelToGame();
}

void APTGameMode::TravelToGame()
{
    if (bIsTraveling)
    {
        return;
    }

    APTGameState* GS = GetGameState<APTGameState>();
    if (GS == nullptr || GS->GetCurrentPhase() != EGamePhase::Waiting)
    {
        return;
    }

    const FString TargetGameMapPath = ResolveGameMapPath();
    if (TargetGameMapPath.IsEmpty())
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

    UE_LOG(LogTemp, Log, TEXT("[Loading] TravelToGame started. TargetMap=%s"), *TargetGameMapPath);

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
    const FString TargetGameMapPath = ResolveGameMapPath();
    if (World == nullptr || TargetGameMapPath.IsEmpty())
    {
        bIsTraveling = false;
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    if (SaveSubsystem == nullptr)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Save] Save subsystem is unavailable before server travel. TargetMap=%s"),
            *TargetGameMapPath);
    }
    else if (!SaveSubsystem->SaveAllAuthorityPlayers(false))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Save] Pre-travel save did not save any authority player. TargetMap=%s"),
            *TargetGameMapPath);
    }
    else
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("[Save] Pre-travel save saved at least one authority player. TargetMap=%s"),
            *TargetGameMapPath);
    }

    World->ServerTravel(TargetGameMapPath + TEXT("?listen"));
}

FString APTGameMode::ResolveGameMapPath() const
{
    if (!GameMapPath.IsEmpty() && !GameMapPath.Equals(LegacyMissingGameMapPath, ESearchCase::IgnoreCase))
    {
        return GameMapPath;
    }

    if (!FallbackGameMapPath.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Loading] GameMapPath is empty or points to legacy missing map. GameMapPath=%s Fallback=%s"),
            *GameMapPath,
            *FallbackGameMapPath);
        return FallbackGameMapPath;
    }

    return GameMapPath;
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

void APTGameMode::ApplyPendingPlayerCharacterData(AController* PlayerController) const
{
    APTBasePlayerState* PlayerState =
        PlayerController != nullptr ? PlayerController->GetPlayerState<APTBasePlayerState>() : nullptr;
    if (PlayerState == nullptr)
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    if (SaveSubsystem != nullptr)
    {
        SaveSubsystem->ApplyPendingPlayerCharacterData(PlayerState);
    }
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
    ApplyPendingPlayerCharacterData(C);
    StartAutoSaveIfAvailable();
}

// [레벨트리거] 태그 기반 스폰 위치 선택
AActor* APTGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    APlayerController* PC = Cast<APlayerController>(Player);
    FString IncomingTag;

    if (PC)
    {
        if (PC->PendingSwapConnection)
        {
            FString CombinedOptions = FString::Join(PC->PendingSwapConnection->URL.Op, TEXT(" "));
            IncomingTag = UGameplayStatics::ParseOption(CombinedOptions, TEXT("PlayerActorTag"));
        }

        if (IncomingTag.IsEmpty() && PC->NetConnection)
        {
            FString CombinedOptions = FString::Join(PC->NetConnection->URL.Op, TEXT(" "));
            IncomingTag = UGameplayStatics::ParseOption(CombinedOptions, TEXT("PlayerActorTag"));
        }

        if (IncomingTag.IsEmpty() && GetWorld())
        {
            IncomingTag = UGameplayStatics::ParseOption(GetWorld()->GetAddressURL(), TEXT("PlayerActorTag"));
        }
    }

    if (IncomingTag.IsEmpty() || IncomingTag.Equals(TEXT("None"), ESearchCase::IgnoreCase))
    {
        IncomingTag = TEXT("Default");
    }

    if (GetWorld())
    {
        for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
        {
            APlayerStart* StartActor = *It;
            if (StartActor && StartActor->PlayerStartTag.ToString() == IncomingTag)
            {
                UE_LOG(LogTemp, Log, TEXT("[PTGameMode] Found matching PlayerStart with Tag: %s"), *IncomingTag);
                return StartActor;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("[PTGameMode] Requested PlayerActorTag '%s' was not found in this level. Falling back to default spawn."), *IncomingTag);
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}

void APTGameMode::RequestLevelTransition(FName LevelName)
{
    if (!HasAuthority() || bIsTraveling)
    {
        return;
    }

    if (LevelName.IsNone())
    {
        return;
    }

    bIsTraveling = true;
    FString TravelURL = LevelName.ToString() + TEXT("?listen");
    GetWorld()->ServerTravel(TravelURL, false, false);
}


void APTGameMode::RestartPlayerAtTransform(AController* PlayerController, const FTransform& SpawnTransform)
{
    if (PlayerController == nullptr)
    {
        return;
    }

    FTransform FinalSpawnTransform = SpawnTransform;
    APTBasePlayerState* PlayerState = PlayerController->GetPlayerState<APTBasePlayerState>();

    if (PlayerState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTDebug] HasRespawnLocation: %s"), PlayerState->HasRespawnLocation() ? TEXT("TRUE") : TEXT("FALSE"));
        if (PlayerState->HasRespawnLocation())
        {
            UE_LOG(LogTemp, Warning, TEXT("[PTDebug] Saved Location: %s"), *PlayerState->GetSavedRespawnLocation().ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[PTDebug] PlayerState is NULL at RestartPlayerAtTransform!"));
    }
    UE_LOG(LogTemp, Warning, TEXT("[PTDebug] SpawnTransform Location: %s"), *SpawnTransform.GetLocation().ToString());

    if (PlayerState != nullptr && PlayerState->HasRespawnLocation())
    {
        bool bIsSpawnedAtTaggedStart = false;

        if (GetWorld())
        {
            for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
            {
                APlayerStart* StartActor = *It;
                if (StartActor)
                {
                    FVector StartLoc = StartActor->GetActorLocation();
                    FVector SpawnLoc = SpawnTransform.GetLocation();

                    float DistanceXY = FVector::DistXY(StartLoc, SpawnLoc);
                    float DistanceZ = FMath::Abs(StartLoc.Z - SpawnLoc.Z);

                    if (DistanceXY < 50.0f && DistanceZ < 100.0f &&
                        !StartActor->PlayerStartTag.IsNone() &&
                        StartActor->PlayerStartTag.ToString() != TEXT("Default"))
                    {
                        bIsSpawnedAtTaggedStart = true;
                        break;
                    }
                }
            }
        }

        if (!bIsSpawnedAtTaggedStart)
        {
            FVector RespawnLocation = PlayerState->GetSavedRespawnLocation();
            FinalSpawnTransform.SetLocation(RespawnLocation);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("[PTGameMode] Tagged Portal travel detected (%s). Bypassing saved checkpoint location."),
                *SpawnTransform.GetLocation().ToString());
        }
    }

    Super::RestartPlayerAtTransform(PlayerController, FinalSpawnTransform);
}

// [★새롭게 추가된 핵심 차단 프로세스] ClientTravel 주소창 파싱 통로
FString APTGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
    // 1. 부모 클래스의 원래 기능을 먼저 안전하게 실행합니다.
    FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

    // 2. ClientTravel로 들어온 Options 주소창 데이터를 월드 URL 세팅에 강제로 반영합니다.
    if (GetWorld() && UGameplayStatics::HasOption(Options, TEXT("PlayerActorTag")))
    {
        // 빈칸이나 ? 기호 기준으로 옵션을 쪼개서 FURL::Op 배열에 넣어줍니다.
        TArray<FString> SplitOptions;
        Options.ParseIntoArray(SplitOptions, TEXT("?"), true);

        for (const FString& Option : SplitOptions)
        {
            if (!Option.IsEmpty())
            {
                GetWorld()->URL.Op.AddUnique(Option);
            }
        }
    }

    return ErrorMessage;
}
