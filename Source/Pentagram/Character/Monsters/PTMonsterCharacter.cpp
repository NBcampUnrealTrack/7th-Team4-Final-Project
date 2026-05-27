#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTMonsterAIController.h"
// #include "Character/Player/PTPlayerCharacter.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionShape.h"

APTMonsterCharacter::APTMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APTMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    SpawnLocation = GetActorLocation();

    InitializeMonster();
}

void APTMonsterCharacter::InitializeMonster()
{
    const FPTCharacterRow* Row = CharacterDataHandle.GetRow<FPTCharacterRow>(TEXT("InitializeMonster"));
    if (!Row) return;

    SightAngle = Row->SightAngle;
    SightRange = Row->SightRange;
    ChaseRange = Row->ChaseRange;
    AttackRange = Row->AttackRange;
    PatrolRadius = Row->PatrolRadius;
    MaxChaseDistance = Row->MaxChaseDistance;
    GoldDropMin = Row->GoldDropMin;
    GoldDropMax = Row->GoldDropMax;
    EquipDropRate = Row->EquipDropRate;
    RewardExp = Row->RewardExp;

    SetMonsterState(EMonsterState::Idle);

    if (APTMonsterAIController* AIC = Cast<APTMonsterAIController>(GetController()))
    {
        AIC->UpdateSightConfig(SightRange, ChaseRange, SightAngle);
        AIC->UpdateMonsterBlackboard(this);
    }
}

void APTMonsterCharacter::SetMonsterState(EMonsterState NewState)
{
    CurrentState = NewState;
}

void APTMonsterCharacter::OnDeath()
{
    if (bIsDead)
    {
        return;
    }
    bIsDead = true;

    Super::OnDeath();

    SetMonsterState(EMonsterState::Dead);

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (AIC->BrainComponent)
        {
            AIC->BrainComponent->StopLogic(TEXT("Monster Dead"));
        }
    }

    if (HasAuthority())
    {
        SpawnDeathDrops();
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && DeathMontage)
    {
        float MontageLength = AnimInstance->Montage_Play(DeathMontage);
        if (MontageLength > 0.f)
        {
            DestroyDelay = MontageLength;
        }
    }

    GetWorldTimerManager().SetTimer(
        DestroyTimerHandle,
        this,
        &APTMonsterCharacter::HandleDestroyAfterDeath,
        DestroyDelay,
        false
    );
}

void APTMonsterCharacter::SpawnDeathDrops()
{
    // 골드 — EconomySubsystem 직접 지급
    // TODO: PTEconomySubsystem 연동 후 구현

    // 경험치 — LevelSubsystem 직접 지급
    // TODO: PTPlayerLevelSubsystem 연동 후 구현

    // 장비 — AItemActorBase 스폰
    // TODO: 아이템 파트 AItemActorBase 구현 완료 후 EquipmentDropClass로 스폰
}

void APTMonsterCharacter::HandleDestroyAfterDeath()
{
    Destroy();
}

void APTMonsterCharacter::PerformAttack()
{
    if (!HasAuthority())
    {
        return;
    }

    HitActors.Empty();

    const FVector TraceStart = GetActorLocation();
    const FVector TraceEnd = TraceStart;
    const float TraceRadius = AttackRange;

    TArray<FHitResult> HitResults;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    const bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults, TraceStart, TraceEnd,
        FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeSphere(TraceRadius), Params
    );

    DrawDebugSphere(GetWorld(), TraceStart, TraceRadius, 16, bHit ? FColor::Green : FColor::Red, false, 1.f);

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!IsValid(HitActor))
        {
            continue;
        }

        if (HitActors.Contains(HitActor))
        {
            continue;
        }

        HitActors.Add(HitActor);

        // TODO 플레이어 캐릭터 합치면 주석 풀어주면 됨
        /*if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(HitActor))
        {
            Player->ApplyDamage(BaseAtk, this);
        }*/
    }
}
