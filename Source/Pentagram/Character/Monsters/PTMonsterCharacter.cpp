#include "Character/Monsters/PTMonsterCharacter.h"

#include "Character/Monsters/AI/PTMonsterAIController.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Core/PTRewardSubsystem.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "CollisionShape.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

APTMonsterCharacter::APTMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
}

float APTMonsterCharacter::ApplyDamage(float DamageAmount, AActor* Attacker)
{
    if (IsDead())
    {
        return 0.f;
    }

    const float FinalDamage = Super::ApplyDamage(DamageAmount, Attacker);

    if (FinalDamage <= 0.f)
    {
        return 0.f;
    }

    if (HasAuthority())
    {
        if (IsValid(Attacker))
        {
            RegisterDamageContributor(Attacker);
        }

        OnHPChanged.Broadcast(CurrentHP, MaxHP);
    }

    return FinalDamage;
}

void APTMonsterCharacter::InitializeMonster()
{
    const FPTCharacterRow* Row = CharacterDataHandle.GetRow<FPTCharacterRow>(TEXT("InitializeMonster"));
    if (!Row)
    {
        return;
    }

    SightAngle       = Row->SightAngle;
    SightRange       = Row->SightRange;
    ChaseRange       = Row->ChaseRange;
    AttackRange      = Row->AttackRange;
    PatrolRadius     = Row->PatrolRadius;
    MaxChaseDistance = Row->MaxChaseDistance;
    GoldDropMin      = Row->GoldDropMin;
    GoldDropMax      = Row->GoldDropMax;
    EquipDropRate    = Row->EquipDropRate;
    RewardExp        = Row->RewardExp;

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
}

void APTMonsterCharacter::PerformAttack()
{
    if (!HasAuthority())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    const FVector TraceStart = GetActorLocation()
        + GetActorForwardVector() * AttackForwardOffset
        + FVector(0.f, 0.f, AttackHeightOffset);
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
            Player->ApplyDamage(GetAttackDamage(), this);
        }
    }
}

float APTMonsterCharacter::StartAttack()
{
    HitActors.Empty();

    if (!IsValid(AttackMontage))
    {
        return 1.f;
    }

    const float Duration = AttackMontage->GetPlayLength();

    if (HasAuthority())
    {
        Multicast_PlayAttackMontage(AttackMontage);
    }

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

void APTMonsterCharacter::Multicast_PlayAttackMontage_Implementation(UAnimMontage* MontageToPlay)
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!IsValid(MeshComp))
    {
        return;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!IsValid(AnimInstance) || !IsValid(MontageToPlay))
    {
        return;
    }

    AnimInstance->Montage_Play(MontageToPlay);

    UE_LOG(LogTemp, Log, TEXT("[Monster] Multicast Play Montage: %s"), *GetNameSafe(MontageToPlay));
}

FPTMonsterRewardData APTMonsterCharacter::GetRewardData() const
{
    FPTMonsterRewardData Data;
    Data.RewardExp          = RewardExp;
    Data.GoldDropMin        = GoldDropMin;
    Data.GoldDropMax        = GoldDropMax;
    Data.EquipDropRate      = EquipDropRate;
    Data.GoldPickupClass    = GoldPickupClass;
    Data.EquipmentDropClass = EquipmentDropClass;
    Data.ItemRowHandle      = ItemRowHandle;

    return Data;
}

void APTMonsterCharacter::ClearExpContributors()
{
    ExpContributors.Empty();
}

void APTMonsterCharacter::OnRep_CurrentState()
{
    if (CurrentState == EMonsterState::Dead)
    {
        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PlayDeathMontage();
    }
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

void APTMonsterCharacter::OnDeath()
{
    if (IsDead())
    {
        return;
    }

    if (!HasAuthority())
    {
        return;
    }

    Super::OnDeath();

    SetMonsterState(EMonsterState::Dead);

    const float MontageLength = PlayDeathMontage();
    const float ActualDelay   = MontageLength > 0.f ? MontageLength : DestroyDelay;

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (IsValid(AIC->BrainComponent))
        {
            AIC->BrainComponent->StopLogic(TEXT("Monster Dead"));
        }
    }

    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        UPTRewardSubsystem* RewardSys = World->GetSubsystem<UPTRewardSubsystem>();
        if (IsValid(RewardSys))
        {
            RewardSys->HandleMonsterDeathReward(this);
        }
    }

    GetWorldTimerManager().SetTimer(
        DestroyTimerHandle,
        this,
        &APTMonsterCharacter::DestroyAfterDeath,
        ActualDelay,
        false
    );
}

float APTMonsterCharacter::GetAttackDamage() const
{
    return BaseAtk;
}

void APTMonsterCharacter::OnRep_CurrentHP()
{
    OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void APTMonsterCharacter::RegisterDamageContributor(AActor* DamageCauser)
{
    if (!IsValid(DamageCauser))
    {
        return;
    }

    APawn* Pawn = Cast<APawn>(DamageCauser);
    if (!IsValid(Pawn))
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!IsValid(PC))
    {
        return;
    }

    APTBasePlayerState* PS = PC->GetPlayerState<APTBasePlayerState>();
    if (!IsValid(PS))
    {
        return;
    }

    ExpContributors.Add(PS);
}

void APTMonsterCharacter::DestroyAfterDeath()
{
    Destroy();
}

float APTMonsterCharacter::PlayDeathMontage()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!IsValid(MeshComp) || !IsValid(DeathMontage))
    {
        return 0.f;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!IsValid(AnimInstance))
    {
        return 0.f;
    }

    if (AnimInstance->Montage_IsPlaying(DeathMontage))
    {
        return 0.f;
    }

    const float PlayResult = AnimInstance->Montage_Play(DeathMontage);

    FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveMontageInstance();
    if (MontageInstance)
    {
        MontageInstance->bEnableAutoBlendOut = false;
    }

    return PlayResult;
}
