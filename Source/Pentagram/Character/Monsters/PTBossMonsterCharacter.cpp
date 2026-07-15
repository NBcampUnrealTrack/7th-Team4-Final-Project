#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Character/Skill/PTBossPatternComponent.h"
#include "Character/Skill/PTMonsterSkillComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Monsters/Skill/PTBossRoomCenter.h"
#include "Character/Monsters/Skill/PTBossShield.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

APTBossMonsterCharacter::APTBossMonsterCharacter()
{
    CharacterType = ECharacterType::BossMonster;

    BossPatternComponent = CreateDefaultSubobject<UPTBossPatternComponent>(TEXT("BossPatternComponent"));

    bIsRanged = true;
    OptimalRangeValue = 800.f;
}

int32 APTBossMonsterCharacter::GetCurrentPhase() const
{
    if (IsDead() || MaxHP <= 0.f)
    {
        return 0;
    }

    const float Phase1EntryRatio = FMath::Max(Phase1HPThreshold, Phase2HPThreshold);
    const float Phase2EntryRatio = FMath::Min(Phase1HPThreshold, Phase2HPThreshold);

    const float HPRatio = CurrentHP / MaxHP;

    if (HPRatio <= Phase2EntryRatio)
    {
        return 2;
    }

    if (HPRatio <= Phase1EntryRatio)
    {
        return 1;
    }

    return 0;
}

float APTBossMonsterCharacter::GetDamageMultiplierForPhase(int32 Phase) const
{
    if (Phase >= 2)
    {
        return Phase2DamageMultiplier;
    }

    if (Phase >= 1)
    {
        return Phase1DamageMultiplier;
    }

    return 1.f;
}

UAnimMontage* APTBossMonsterCharacter::GetAttackMontageForPhase(int32 Phase) const
{
    if (Phase >= 2 && IsValid(BerserkAttackMontage))
    {
        return BerserkAttackMontage;
    }

    if (Phase >= 1 && IsValid(EnragedAttackMontage))
    {
        return EnragedAttackMontage;
    }

    if (!IsValid(AttackMontage))
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[BossMonster] AttackMontage가 설정되지 않았습니다."));
#endif
    }

    return AttackMontage;
}

void APTBossMonsterCharacter::OnShieldDestroyed()
{
    GetWorldTimerManager().ClearTimer(ShieldTimerHandle);
    ActiveShield = nullptr;

    UnfreezeAfterShieldPhase();
}

void APTBossMonsterCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (IsValid(BossPatternComponent) && IsValid(SkillComponent))
    {
        BossPatternComponent->SetSkillComponent(SkillComponent);
        BossPatternComponent->PreloadAllSkills();
    }
}

void APTBossMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    OnPhaseChanged.AddUniqueDynamic(this, &APTBossMonsterCharacter::TryEnterShieldPhase);
}

void APTBossMonsterCharacter::OnDeath()
{
    if (IsValid(BossPatternComponent))
    {
        BossPatternComponent->ClearProjectileTimers();
        BossPatternComponent->StopLaser();
    }

    GetWorldTimerManager().ClearTimer(ShieldTimerHandle);
    if (IsValid(ActiveShield))
    {
        ActiveShield->SuppressDestroyNotify();
        ActiveShield->Destroy();
        ActiveShield = nullptr;
    }

    Super::OnDeath();
}

void APTBossMonsterCharacter::PerformAttack()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!IsValid(BossPatternComponent) || !IsValid(BossPatternComponent->BossSkillDataTable) || !IsValid(SkillComponent))
    {
        Super::PerformAttack();
        return;
    }

    const FName CurrentSkill = SkillComponent->GetCurrentSkillID();
    const FPTBossSkillRow* Row = BossPatternComponent->BossSkillDataTable->FindRow<FPTBossSkillRow>(
        CurrentSkill, TEXT("BossPerformAttack"));

    if (!Row)
    {
        Super::PerformAttack();
        return;
    }

    switch (Row->SkillType)
    {
    case EBossSkillType::Projectile:
    case EBossSkillType::Area:
        return;

    case EBossSkillType::Melee:
    default:
    {
        UWorld* World = GetWorld();
        if (!IsValid(World)) return;

        const FVector TraceStart = GetActorLocation()
            + GetActorForwardVector() * AttackForwardOffset
            + FVector(0.f, 0.f, AttackHeightOffset);

        TArray<FHitResult> HitResults;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        World->SweepMultiByChannel(HitResults, TraceStart, TraceStart,
            FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(AttackRadius), Params);

        const float FinalDamage = GetBaseAtk()
            * Row->DamageMultiplier
            * GetDamageMultiplierForPhase(GetCurrentPhase());

        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (!IsValid(HitActor) || HitActors.Contains(HitActor)) continue;
            HitActors.Add(HitActor);

            if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(HitActor))
            {
                FPTHitInfo HitInfo = Row->MakeHitInfo(this);
                HitInfo.HitDirection = GetActorForwardVector();
                Player->ApplyDamageWithHit(FinalDamage, this, HitInfo);
            }
        }
    }
    break;
    }
}

void APTBossMonsterCharacter::SetMeleeAttackData(float Damage, const FPTHitInfo& HitInfo)
{
    CurrentMeleeDamage = Damage;
    CurrentMeleeHitInfo = HitInfo;
    HitActorsThisSwing.Empty();
}

void APTBossMonsterCharacter::ClearMeleeAttackData()
{
    CurrentMeleeDamage = 0.f;
    CurrentMeleeHitInfo = FPTHitInfo();
    HitActorsThisSwing.Empty();
}

bool APTBossMonsterCharacter::IsAlreadyHit(TWeakObjectPtr<AActor> Target) const
{
    return HitActorsThisSwing.Contains(Target);
}

void APTBossMonsterCharacter::AddHitActor(TWeakObjectPtr<AActor> Target)
{
    HitActorsThisSwing.Add(Target);
}

void APTBossMonsterCharacter::ClearHitActors()
{
    HitActorsThisSwing.Empty();
}

void APTBossMonsterCharacter::MulticastPlayHitSound_Implementation(FVector Location, USoundBase* Sound)
{
    if (!IsValid(Sound))
    {
        return;
    }

    UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location);
}

float APTBossMonsterCharacter::StartAttack()
{
    HitActors.Empty();

    const int32 Phase = GetCurrentPhase();

    if (IsValid(BossPatternComponent))
    {
        const float SkillPlayLength = BossPatternComponent->ExecuteSkillForPhase(Phase);
        if (SkillPlayLength > 0.f)
        {
            if (BossPatternComponent->HasPendingSkill())
            {
                const FPTBossSkillRow* Snapshot = BossPatternComponent->GetPendingSkillSnapshot();
                if (Snapshot->bRequiresCenterMove && IsValid(RoomCenterActor))
                {
                    SetActorLocation(RoomCenterActor->GetActorLocation());
                }

                if (Snapshot->bLockMovementDuringAttack)
                {
                    ApplyAttackMovementLock();
                }
            }

            return SkillPlayLength;
        }
    }

    UAnimMontage* Montage = GetAttackMontageForPhase(GetCurrentPhase());
    if (IsValid(Montage))
    {
        ApplyAttackMovementLock();
        return Montage->GetPlayLength();
    }

    return 1.f;
}

void APTBossMonsterCharacter::StopAttack()
{
    RestoreAttackMovementLock();

    if (IsValid(BossPatternComponent))
    {
        BossPatternComponent->ClearProjectileTimers();
        BossPatternComponent->StopLaser();
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!IsValid(MeshComp))
    {
        return;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!IsValid(AnimInstance))
    {
        return;
    }

    if (IsValid(BerserkAttackMontage) && AnimInstance->Montage_IsPlaying(BerserkAttackMontage))
    {
        AnimInstance->Montage_Stop(0.f, BerserkAttackMontage);
    }
    else if (IsValid(EnragedAttackMontage) && AnimInstance->Montage_IsPlaying(EnragedAttackMontage))
    {
        AnimInstance->Montage_Stop(0.f, EnragedAttackMontage);
    }
    else
    {
        Super::StopAttack();
    }
}

float APTBossMonsterCharacter::GetAttackDamage() const
{
    return BaseAtk * GetDamageMultiplierForPhase(GetCurrentPhase());
}

void APTBossMonsterCharacter::TryEnterShieldPhase(int32 NewPhase)
{
    if (!bHasShieldPhase || bShieldPhaseTriggered || NewPhase < 1 || !HasAuthority())
    {
        return;
    }

    bShieldPhaseTriggered = true;
    SpawnShield();
}

void APTBossMonsterCharacter::SpawnShield()
{
    if (!IsValid(ShieldClass))
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.Owner      = this;
    Params.Instigator = GetInstigator();

    ActiveShield = World->SpawnActor<APTBossShield>(
        ShieldClass, GetActorLocation(), FRotator::ZeroRotator, Params
    );

    if (!IsValid(ActiveShield))
    {
        return;
    }

    ActiveShield->InitShield(this, ShieldMaxHP, ShieldRadius, ShieldPushForce);

    FreezeForShieldPhase();

    GetWorldTimerManager().SetTimer(ShieldTimerHandle, this, &APTBossMonsterCharacter::OnShieldTimerExpired, ShieldDuration, false);
}

void APTBossMonsterCharacter::OnShieldTimerExpired()
{
    if (!HasAuthority() || !IsValid(ActiveShield))
    {
        return;
    }

    const float Recovered = FMath::Min(ActiveShield->GetCurrentShieldHP(), MaxHP - CurrentHP);
    if (Recovered > 0.f)
    {
        CurrentHP += Recovered;
        OnHPChanged.Broadcast(CurrentHP, MaxHP);
    }

    ActiveShield->SuppressDestroyNotify();
    ActiveShield->Destroy();
    ActiveShield = nullptr;

    UnfreezeAfterShieldPhase();
}

void APTBossMonsterCharacter::FreezeForShieldPhase()
{
    if (IsValid(BossPatternComponent))
    {
        BossPatternComponent->ClearProjectileTimers();
        BossPatternComponent->StopLaser();
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->SetMovementMode(MOVE_None);
    }
    
    AAIController* AIC = Cast<AAIController>(GetController());
    if (IsValid(AIC))
    {
        AIC->StopMovement();
        if (IsValid(AIC->BrainComponent))
        {
            AIC->BrainComponent->PauseLogic(TEXT("ShieldPhase"));
        }
    }
}

void APTBossMonsterCharacter::UnfreezeAfterShieldPhase()
{
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->SetMovementMode(MOVE_Walking);
    }

    AAIController* AIC = Cast<AAIController>(GetController());
    if (IsValid(AIC) && IsValid(AIC->BrainComponent))
    {
        AIC->BrainComponent->ResumeLogic(TEXT("ShieldPhase"));
    }
}
