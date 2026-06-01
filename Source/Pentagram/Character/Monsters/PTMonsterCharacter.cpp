#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTMonsterAIController.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionShape.h"
#include "Core/PTEconomySubsystem.h"
#include "Core/PTPlayerLevelSubsystem.h"
#include "Net/UnrealNetwork.h"

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

void APTMonsterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(DestroyTimerHandle);

    Super::EndPlay(EndPlayReason);
}

void APTMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTMonsterCharacter, CurrentState);
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
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;

    if (CurrentState == EMonsterState::Dead)
    {
        PlayDeathMontage();
    }
}

void APTMonsterCharacter::OnDeath()
{
    if (IsDead())
    {
        return;
    }

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
        GetWorldTimerManager().SetTimer(
            DestroyTimerHandle,
            this,
            &APTMonsterCharacter::HandleDestroyAfterDeath,
            DestroyDelay,
            false
        );
    }
}

void APTMonsterCharacter::SpawnDeathDrops()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        return;
    }

    if (UPTEconomySubsystem* EconomySys = GI->GetSubsystem<UPTEconomySubsystem>())
    {
        const int32 Gold = FMath::RandRange(GoldDropMin, GoldDropMax);
        EconomySys->AddGold(Gold);
    }

    if (UPTPlayerLevelSubsystem* LevelSys = GI->GetSubsystem<UPTPlayerLevelSubsystem>())
    {
        LevelSys->AddExp(RewardExp);
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (EquipmentDropClass && FMath::FRand() <= EquipDropRate)
    {
        const FVector DropLocation = GetActorLocation() + FVector(0.f, 0.f, 50.f);
        World->SpawnActor<AActor>(
            EquipmentDropClass,
            DropLocation,
            FRotator::ZeroRotator
        );
    }
}

void APTMonsterCharacter::HandleDestroyAfterDeath()
{
    Destroy();
}

void APTMonsterCharacter::PlayDeathMontage()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp || !DeathMontage)
    {
        return;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }

    if (AnimInstance->Montage_IsPlaying(DeathMontage))
    {
        return;
    }

    const float MontageLength = AnimInstance->Montage_Play(DeathMontage);
    if (MontageLength > 0.f)
    {
        DestroyDelay = MontageLength;
    }
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

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const bool bHit = World->SweepMultiByChannel(
        HitResults, TraceStart, TraceEnd,
        FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeSphere(TraceRadius), Params
    );

#if !UE_BUILD_SHIPPING
    DrawDebugSphere(World, TraceStart, TraceRadius, 16, bHit ? FColor::Green : FColor::Red, false, 1.f);
#endif

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

        if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(HitActor))
        {
            Player->ApplyDamage(GetAttackDamage(), this);
        }
    }
}

float APTMonsterCharacter::StartAttack()
{
    HitActors.Empty();

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!IsValid(MeshComp))
    {
        return 1.f;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!IsValid(AnimInstance) || !IsValid(AttackMontage))
    {
        return 1.f;
    }

    const float Duration = AnimInstance->Montage_Play(AttackMontage);

    return Duration > 0.f ? Duration : 1.f;
}

void APTMonsterCharacter::StopAttack()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!IsValid(MeshComp))
    {
        return;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (IsValid(AnimInstance) && IsValid(AttackMontage))
    {
        AnimInstance->Montage_Stop(0.f, AttackMontage);
    }
}

float APTMonsterCharacter::GetAttackDamage() const
{
    return BaseAtk;
}

void APTMonsterCharacter::OnRep_CurrentState()
{
    if (CurrentState == EMonsterState::Dead)
    {
        PlayDeathMontage();
    }
}
