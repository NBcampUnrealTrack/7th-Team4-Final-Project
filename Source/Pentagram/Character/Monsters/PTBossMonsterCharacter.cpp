#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Animation/AnimInstance.h"

APTBossMonsterCharacter::APTBossMonsterCharacter()
{

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

UAnimMontage* APTBossMonsterCharacter::GetAttackMontageForPhase(int32 Phase) const
{
    if (Phase >= 2 && BerserkAttackMontage)
    {
        return BerserkAttackMontage;
    }

    if (Phase >= 1 && EnragedAttackMontage)
    {
        return EnragedAttackMontage;
    }

    if (!AttackMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossMonster] AttackMontage가 설정되지 않았습니다."));
    }

    return AttackMontage;
}

float APTBossMonsterCharacter::StartAttack()
{
    HitActors.Empty();

    const int32 Phase = GetCurrentPhase();
    UAnimMontage* Montage = GetAttackMontageForPhase(Phase);
    if (!Montage)
    {
        return 1.f;
    }

    const float Duration = Montage->GetPlayLength();

    if (HasAuthority())
    {
        Multicast_PlayAttackMontage(Montage);
    }

    return Duration > 0.f ? Duration : 1.f;
}

void APTBossMonsterCharacter::StopAttack()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }

    if (BerserkAttackMontage && AnimInstance->Montage_IsPlaying(BerserkAttackMontage))
    {
        AnimInstance->Montage_Stop(0.f, BerserkAttackMontage);
    }
    else if (EnragedAttackMontage && AnimInstance->Montage_IsPlaying(EnragedAttackMontage))
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
