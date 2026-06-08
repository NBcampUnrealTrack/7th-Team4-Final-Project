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
                // 체크포인트가 있다면 그 지점에서 부활 
                // 리스폰 지점이 다른 지형지물에 겹쳐서 캐릭터가 낀 채로 부활하는 상황을 방지하기 위한 변수 
                FVector FinalSpawnLoc = RespawnLoc;
                bool bFoundSafeSpot = false;

                // 캐릭터의 크기에 맞춰서 주변 빈 공간을 탐색하기 위한 가상의 캡슐 형태 정의
                float TestRadius = 45.0f;
                float TestHalfHeight = 95.0f;
                FCollisionShape CharacterShape = FCollisionShape::MakeCapsule(TestRadius, TestHalfHeight);

                FCollisionQueryParams QueryParams;
                QueryParams.AddIgnoredActor(this);

                // 주변 무작위 빈 공간 탐색 시도 (최대 10번 빙글빙글 돌며 스캔)
                for (int32 i = 0; i < 10; ++i)
                {
                    FVector RandomOffset = FVector::ZeroVector;

                    if (i > 0) // 첫 시도(i=0)에는 정중앙 위를 먼저 보고, 꽉 막혔다면 주변을 탐색
                    {
                        // 리스폰 포인트 기준 평면(X, Y) 상으로 150~250 유닛 떨어진 랜덤한 방향 계산
                        float RandomAngle = FMath::FRandRange(0.0f, 360.0f);
                        float RandomDistance = FMath::FRandRange(150.0f, 250.0f);

                        RandomOffset.X = FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance;
                        RandomOffset.Y = FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance;
                        // 땅에 파묻히지 않도록 높이 보정치 추가
                        RandomOffset.Z = 50.0f;
                    }

                    FVector TestLoc = RespawnLoc + RandomOffset;
                    TestLoc.Z += TestHalfHeight; // 캡슐의 중심점 높이로 정렬

                    // 해당 위치에 장애물이나 벽, 리스폰 포인트 구조물이 겹치는지 엔진 장부 스캔
                    bool bOverlap = GetWorld()->OverlapBlockingTestByChannel(
                        TestLoc,
                        FQuat::Identity,
                        ECC_Pawn, // 캐릭터들이 충돌하는 채널 기준으로 스캔
                        CharacterShape,
                        QueryParams
                    );

                    if (!bOverlap)
                    {
                        // 아무것도 겹치지 않는 완전 안전한 빈 자리를 찾았음
                        FinalSpawnLoc = TestLoc;
                        bFoundSafeSpot = true;
                        break;
                    }
                }

                if (!bFoundSafeSpot)
                {
                    // 만약 주변 10개 구역이 전부 무언가로 꽉 막혀있다면 최후의 수단으로 공중 부활시킴 
                    FinalSpawnLoc = RespawnLoc;
                    FinalSpawnLoc.Z += 300.0f;
                    UE_LOG(LogTemp, Warning, TEXT("주변 빈 공간을 찾지 못해 강제로 높은 공중에서 부활시킵니다."));
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("리스폰 포인트 주변의 안전한 빈 자리를 확보했습니다: %s"), *FinalSpawnLoc.ToString());
                }

                // 빈 공간 검증을 마쳤으므로, 스폰할 때 아주 미세하게 충돌이 밀려나도 무조건 스폰되게 옵션 변경
                FActorSpawnParameters SpawnParams; 
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                SpawnParams.Owner = this;

                if (DefaultPawnClass)
                {
                    APawn* NewPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FinalSpawnLoc, FRotator::ZeroRotator, SpawnParams);
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
