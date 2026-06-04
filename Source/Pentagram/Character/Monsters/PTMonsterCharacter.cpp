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
#include "Core/PTPlayerLevelSubsystem.h"
#include "Character/Player/PTBasePlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Item/PTGoldPickup.h"
#include "Net/UnrealNetwork.h"
#include "Core/PTRewardSubsystem.h"

APTMonsterCharacter::APTMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
}

float APTMonsterCharacter::ApplyDamage(float DamageAmount, AActor* Attacker)
{
    const float FinalDamage = Super::ApplyDamage(DamageAmount, Attacker);

    if (FinalDamage > 0.f && HasAuthority())
    {
        RegisterDamageContributor(Attacker);
    }

    return FinalDamage;
}

void APTMonsterCharacter::ClearExpContributors()
{
    ExpContributors.Empty();
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
    if (HasAuthority())
    {
        SetMonsterState(EMonsterState::Dead);

        if (AAIController* AIC = Cast<AAIController>(GetController()))
        {
            if (AIC->BrainComponent)
            {
                AIC->BrainComponent->StopLogic(TEXT("Monster Dead"));
            }
        }

        UWorld* World = GetWorld();
        if (World)
        {
            UPTRewardSubsystem* RewardSys = World->GetSubsystem<UPTRewardSubsystem>();
            if (RewardSys)
            {
                RewardSys->HandleMonsterDeathReward(this);
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
}

void APTMonsterCharacter::RegisterDamageContributor(AActor* DamageCauser)
{
    if (!DamageCauser)
    {
        return;
    }

    APawn* Pawn = Cast<APawn>(DamageCauser);
    if (!Pawn)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC)
    {
        return;
    }

    APTBasePlayerState* PS = PC->GetPlayerState<APTBasePlayerState>();
    if (!PS)
    {
        return;
    }

    ExpContributors.Add(PS);
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

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector TraceStart = GetActorLocation() + GetActorForwardVector() * AttackForwardOffset + FVector(0.f, 0.f, AttackHeightOffset);
    const FVector TraceEnd = TraceStart;

    TArray<FHitResult> HitResults;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    const bool bHit = World->SweepMultiByChannel(
        HitResults, TraceStart, TraceEnd,
        FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeSphere(AttackRadius), Params
    );

#if !UE_BUILD_SHIPPING
    DrawDebugSphere(World, TraceStart, AttackRadius, 16, bHit ? FColor::Green : FColor::Red, false, 1.f);
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
            Player->ApplyDamage(GetAttackDamage(), Player);
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

FPTMonsterRewardData APTMonsterCharacter::GetRewardData() const
{
    FPTMonsterRewardData Data;
    Data.RewardExp = RewardExp;
    Data.GoldDropMin = GoldDropMin;
    Data.GoldDropMax = GoldDropMax;
    Data.EquipDropRate = EquipDropRate;
    Data.GoldPickupClass = GoldPickupClass;
    Data.EquipmentDropClass = EquipmentDropClass;

    return Data;
}
