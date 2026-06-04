// PTGameMode.cpp

#include "PTGameMode.h"

#include "Character/Player/PTBasePlayerState.h"
#include "PTGameState.h"
#include "PTPlayerLevelSubsystem.h"
#include "PTQuestSubsystem.h"
#include "PTSaveSubsystem.h"
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
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    QuestSubsystem->SetQuestDataTable(QuestDataTable);
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

void APTGameMode::RespawnPlayer(AController* NewPlayer, const FVector& RespawnLoc, bool bHasCheckpoint)
{
    if (!NewPlayer)
    {
        return;
    }

    FTimerHandle RespawnTimerHandle;

    // 죽는 순간 가로챈 RespawnLoc와 bHasCheckpoint를 3초 뒤 람다 함수로 그대로 안전하게 전달합니다.
    GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, [this, NewPlayer, RespawnLoc, bHasCheckpoint]()
        {
            if (!NewPlayer) return;

            if (bHasCheckpoint)
            {
                FVector SpawnLoc = RespawnLoc;
                SpawnLoc.Z += 150.0f; // 안전 공중 부활 보정

                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                SpawnParams.Owner = this;

                if (DefaultPawnClass)
                {
                    APawn* NewPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
                    if (NewPawn)
                    {
                        NewPlayer->Possess(NewPawn);
                        UE_LOG(LogTemp, Warning, TEXT("유저가 등록된 체크포인트 지점에서 부활했습니다"));
                        return;
                    }
                }
            }

            // 체크포인트가 없다면 기본 스타트 지점으로 부활
            UE_LOG(LogTemp, Log, TEXT("등록된 체크포인트가 없어 초기 스타트 지점에서 부활합니다."));
            this->RestartPlayer(NewPlayer);

        }, 3.0f, false);
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

// 💡 엔진이 최종적으로 캐릭터를 스폰할 위치를 연산할 때 인터셉트하는 정석 오버라이드 함수
void APTGameMode::RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform)
{
    if (NewPlayer == nullptr) return;

    // 1. 부활하려는 유저의 영구 데이터 장부(PlayerState)를 가져옵니다.
    APTBasePlayerState* PS = NewPlayer->GetPlayerState<APTBasePlayerState>();

    // 2. 만약 장부가 존재하고, 저장된 체크포인트 리스폰 지점 좌표가 있다면?
    if (PS && PS->HasRespawnLocation())
    {
        FVector RespawnLoc = PS->GetSavedRespawnLocation();
        RespawnLoc.Z += 150.0f; // 끼임 방지, 공중 스폰

        FTransform CustomSpawnTransform = SpawnTransform;
        CustomSpawnTransform.SetLocation(RespawnLoc);

        // 폰을 스폰할 때 충돌을 완전히 무시하고 무조건 스폰하도록 파라미터를 강제 세팅 
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.Owner = this;

        // 게임모드에 설정된 DefaultPawnClass가 유효한지 검사
        if (DefaultPawnClass)
        {
            // 1. 월드에 캐릭터를 강제로 스폰시킵니다.
            APawn* NewPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, RespawnLoc, FRotator::ZeroRotator, SpawnParams);
            if (NewPawn)
            {
                // 2. 스폰된 캐릭터에 플레이어 컨트롤러를 빙의(Possess)시킵니다.
                NewPlayer->Possess(NewPawn);

                UE_LOG(LogTemp, Log, TEXT("스폰 성공! 유저가 체크포인트에서 부활했습니다."), *NewPlayer->GetName());
                return;
            }
        }
    } 

    // 4. 저장된 체크포인트가 없는 유저라면 원래 설계된 기본 PlayerStart 위치에서 태어나게 둡니다.
    Super::RestartPlayerAtTransform(NewPlayer, SpawnTransform); 
}
