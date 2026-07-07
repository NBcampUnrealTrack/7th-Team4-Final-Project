#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Character/Skill/PTBossPatternComponent.h"
#include "Character/Skill/PTMonsterSkillComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Monsters/Skill/PTBossRoomCenter.h"

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
