#include "Character/Monsters/PTMonsterCharacter.h"

#include "Character/Monsters/AI/PTMonsterAIController.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Core/Subsystems/PTRewardSubsystem.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "CollisionShape.h"
#include "Character/Player/PTPlayerController.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Character/Skill/PTMonsterSkillComponent.h"
#include "Character/PTCombatTypes.h"
#include "Character/Monsters/Projectile/PTBossProjectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"

APTMonsterCharacter::APTMonsterCharacter()
{
    CharacterType = ECharacterType::NormalMonster;
    PrimaryActorTick.bCanEverTick = false;

    SkillComponent = CreateDefaultSubobject<UPTMonsterSkillComponent>(TEXT("SkillComponent"));
}

float APTMonsterCharacter::ApplyDamage(float DamageAmount, AActor* Attacker)
{
    if (HasAuthority() && IsValid(Attacker))
    {
        RegisterDamageContributor(Attacker);
    }

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
            APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(Attacker);
            if (Player)
            {
                APTPlayerController* PC = Cast<APTPlayerController>(Player->GetController());

                if (PC)
                {
                    PC->Client_ShowMonsterHealth(this);
                }
            }
        }

        OnHPChanged.Broadcast(CurrentHP, MaxHP);
    }

    return FinalDamage;
}

float APTMonsterCharacter::ApplyDamageWithHit(float DamageAmount, AActor* Attacker, const FPTHitInfo& HitInfo)
{
    if (HasAuthority() && IsValid(Attacker))
    {
        RegisterDamageContributor(Attacker);
    }

    if (IsDead())
    {
        return 0.f;
    }

    const float FinalDamage = Super::ApplyDamageWithHit(DamageAmount, Attacker, HitInfo);
    if (FinalDamage <= 0.f)
    {
        return 0.f;
    }

    if (HasAuthority())
    {
        if (IsValid(Attacker))
        {
            if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(Attacker))
            {
                if (APTPlayerController* PC = Cast<APTPlayerController>(Player->GetController()))
                {
                    PC->Client_ShowMonsterHealth(this);
                }
            }
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

    if (APTBaseAIController* AIC = Cast<APTBaseAIController>(GetController()))
    {
        AIC->UpdateSightConfig(SightRange, ChaseRange, SightAngle);

        if (APTMonsterAIController* MonsterAIC = Cast<APTMonsterAIController>(AIC))
        {
            MonsterAIC->UpdateMonsterBlackboard(this);
        }
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

    if (bIsRanged && ProjectileClass)
    {
        AActor* Target = nullptr;
        if (AAIController* AIC = Cast<AAIController>(GetController()))
        {
            if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
            {
                Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
            }
        }

        if (!IsValid(Target))
        {
            return;
        }

        const FVector SpawnLoc = GetMesh()->DoesSocketExist(ProjectileSocketName)
            ? GetMesh()->GetSocketLocation(ProjectileSocketName) + GetActorForwardVector() * 80.f
            : GetActorLocation() + GetActorForwardVector() * 120.f + FVector(0.f, 0.f, 50.f);

        const FVector Direction = (Target->GetActorLocation() - SpawnLoc).GetSafeNormal();

        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        APTBossProjectile* Proj = World->SpawnActor<APTBossProjectile>(ProjectileClass, SpawnLoc, Direction.Rotation(), Params);
        if (IsValid(Proj))
        {
            Proj->IgnoreActor(this);
            FPTHitInfo HitInfo;
            float Speed = ProjectileSpeed;
            float Damage = GetAttackDamage();
            if (IsValid(SkillComponent) && IsValid(SkillComponent->SkillDataTable))
            {
                const FPTSkillRow* Row = SkillComponent->GetSkillData(SkillComponent->GetCurrentSkillID());
                if (Row)
                {
                    HitInfo              = Row->MakeHitInfo(this);
                    HitInfo.HitDirection = Direction;
                    Speed                = Row->ProjectileSpeed;
                    Damage              *= Row->DamageMultiplier;
                }
            }

            Proj->Launch(Direction, Damage, Speed, HitInfo);
        }

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
            FPTHitInfo HitInfo;
            float Damage = GetAttackDamage();

            if (IsValid(SkillComponent) && IsValid(SkillComponent->SkillDataTable))
            {
                const FPTSkillRow* Row = SkillComponent->GetSkillData(SkillComponent->GetCurrentSkillID());
                if (Row)
                {
                    HitInfo              = Row->MakeHitInfo(this);
                    HitInfo.HitDirection = GetActorForwardVector();
                    Damage              *= Row->DamageMultiplier;
                }
            }

            Player->ApplyDamageWithHit(Damage, this, HitInfo);
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

    const bool bActivated = SkillComponent->PerformBasicAttack();
    if (!bActivated)
    {
        return 0.f;
    }

    ApplyAttackMovementLock();

    if (IsValid(SkillComponent->SkillDataTable))
    {
        const FPTSkillRow* Row = SkillComponent->SkillDataTable->FindRow<FPTSkillRow>(SkillComponent->BasicAttackRowName, TEXT("StartAttack"));
        if (Row)
        {
            UAnimMontage* Montage = Row->SkillMontage.Get();
            if (!IsValid(Montage))
            {
                Montage = Row->SkillMontage.LoadSynchronous();
            }
            if (IsValid(Montage))
            {
                return Montage->GetPlayLength();
            }
        }
    }

    return 1.f;
}

void APTMonsterCharacter::StopAttack()
{
    SetSuperArmor(false);
    HitActors.Empty();

    RestoreAttackMovementLock();

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

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Log, TEXT("[Monster] Multicast Play Montage: %s"), *GetNameSafe(MontageToPlay));
#endif
}

FPTMonsterRewardData APTMonsterCharacter::GetRewardData() const
{
    FPTMonsterRewardData Data;
    Data.RewardExp          = RewardExp;
    Data.GoldDropMin        = GoldDropMin;
    Data.GoldDropMax        = GoldDropMax;
    Data.ItemDropRate       = EquipDropRate;
    Data.GoldPickupClass    = GoldPickupClass;
    Data.EquipmentDropClass = EquipmentDropClass;
    Data.ItemRowHandle      = ItemRowHandle;
    Data.ItemDropPool       = ItemDropPool;

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
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            Movement->DisableMovement();
        }

        if (UCapsuleComponent* Capsule = GetCapsuleComponent())
        {
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        PlayDeathMontage();
    }
}

void APTMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    SpawnLocation = GetActorLocation();

    InitializeMonster();

    if (SkillComponent)
    {
        SkillComponent->AssignSkillToSlot(SkillComponent->BasicAttackRowName, 0);
    }

    if (SkillComponent && SkillComponent->SkillDataTable)
    {
        FPTSkillRow* Row = SkillComponent->SkillDataTable->FindRow<FPTSkillRow>(SkillComponent->BasicAttackRowName, TEXT(""));
        if (Row)
        {
            Row->SkillMontage.LoadSynchronous();
            Row->SkillEffect.LoadSynchronous();
            Row->SkillSound.LoadSynchronous();
        }
    }
}

void APTMonsterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
    GetWorldTimerManager().ClearTimer(StaggerResumeTimerHandle);

    Super::EndPlay(EndPlayReason);
}

void APTMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTMonsterCharacter, CurrentState);
}

void APTMonsterCharacter::OnDeath()
{
    if (HasAuthority())
    {
        OnMonsterDied.Broadcast(this);
    }

    if (IsDead())
    {
        return;
    }

    if (!HasAuthority())
    {
        return;
    }

    SetSuperArmor(false);
    HitActors.Empty();
    GetWorldTimerManager().ClearTimer(StaggerResumeTimerHandle);

    Super::OnDeath();

    SetMonsterState(EMonsterState::Dead);

    StopAttack();

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement     = false;
        MoveComp->bUseControllerDesiredRotation = false;
    }

    const float MontageLength = PlayDeathMontage();
    const float ActualDelay   = MontageLength > 0.f ? MontageLength + DestroyDelayAfterMontage : DestroyDelay;

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

void APTMonsterCharacter::Multicast_PlayHitReactionMontage_Implementation(EHitReactionType ReactionType)
{
    if (bHasSuperArmor)
    {
        return;
    }

    Super::Multicast_PlayHitReactionMontage_Implementation(ReactionType);
}

void APTMonsterCharacter::ApplyHit(const FPTHitInfo& HitInfo)
{
    if (bHasSuperArmor)
    {
        if (APTBaseCharacter* AttackerChar = Cast<APTBaseCharacter>(HitInfo.Attacker))
        {
            AttackerChar->RequestHitStop(HitInfo.HitStopDuration * 0.5f);
        }

        return;
    }

    if (HitInfo.StaggerDuration > 0.f)
    {
        RestartBTAfterStagger(HitInfo.StaggerDuration);
    }

    Super::ApplyHit(HitInfo);
}

void APTMonsterCharacter::RestartBTAfterStagger(float Duration)
{
    AAIController* AIC = Cast<AAIController>(GetController());
    if (!IsValid(AIC) || !IsValid(AIC->BrainComponent))
    {
        return;
    }

    AIC->BrainComponent->PauseLogic(TEXT("Stagger"));

    GetWorldTimerManager().ClearTimer(StaggerResumeTimerHandle);
    GetWorldTimerManager().SetTimer(
        StaggerResumeTimerHandle,
        this, &APTMonsterCharacter::OnStaggerEnd,
        Duration, false
    );
}

void APTMonsterCharacter::OnStaggerEnd()
{
    if (IsDead())
    {
        return;
    }

    AAIController* AIC = Cast<AAIController>(GetController());
    if (!IsValid(AIC) || !IsValid(AIC->BrainComponent))
    {
        return;
    }

    if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsPatternActive, false);
    }

    AIC->BrainComponent->ResumeLogic(TEXT("Stagger"));
    AIC->StopMovement();
    AIC->BrainComponent->RestartLogic();
}

void APTMonsterCharacter::ApplyAttackMovementLock()
{
    if (bAppliedMovementLock)
    {
        return;
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        bSavedOrientRotationToMovement = MoveComp->bOrientRotationToMovement;
        bAppliedMovementLock = true;

        MoveComp->StopMovementImmediately();
        MoveComp->bOrientRotationToMovement = false;
    }

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        AIC->StopMovement();
    }
}

void APTMonsterCharacter::RestoreAttackMovementLock()
{
    if (!bAppliedMovementLock)
    {
        return;
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = bSavedOrientRotationToMovement;
    }

    bAppliedMovementLock = false;
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

    AnimInstance->StopAllMontages(0.1f);
    const float PlayResult = AnimInstance->Montage_Play(DeathMontage);

    FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveMontageInstance();
    if (MontageInstance)
    {
        MontageInstance->bEnableAutoBlendOut = false;
    }

    return PlayResult;
}
