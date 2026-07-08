#include "PTAnimNotify_FireProjectile.h"

#include "Character/Player/PTPlayerProjectileActor.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTSkillRow.h"
#include "Character/Skill/PTPlayerSkillComponent.h"

void UPTAnimNotify_FireProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!Owner) return;

    if (UWorld* World = Owner->GetWorld())
    {
        const float Now = World->GetTimeSeconds();
        if (const float* LastTime = LastFireTimeByOwner.Find(Owner))
        {
            if (Now - *LastTime < MinRefireInterval)
            {
                UE_LOG(LogTemp, Warning, TEXT("[PTAnimNotify_FireProjectile] 중복 호출 감지, 무시함 (Owner: %s)"), *Owner->GetName());
                return;
            }
        }
        LastFireTimeByOwner.Add(Owner, Now);
    }

    UPTPlayerSkillComponent* SkillComp = Owner->SkillComp;
    FPTSkillRow* SkillData = SkillComp ? SkillComp->GetSkillData(SkillRowName) : nullptr;

    if (!SkillData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTAnimNotify_FireProjectile] SkillRowName '%s'에 해당하는 DT 로우를 찾을 수 없음"),
            *SkillRowName.ToString());
        return;
    }

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    FVector Offset = Owner->GetActorForwardVector() * SkillData->SkillOffset.X
                    + Owner->GetActorRightVector() * SkillData->SkillOffset.Y
                    + FVector(0.f, 0.f, SkillData->SkillOffset.Z);

    FVector SpawnPos   = Owner->GetActorLocation() + Offset;
    FVector ForwardDir = Owner->GetActorForwardVector();
    ForwardDir.Z = 0.f;
    ForwardDir.Normalize();

    UNiagaraSystem* Effect = !ProjectileEffectOverride.IsNull()
        ? ProjectileEffectOverride.LoadSynchronous()
        : SkillData->ProjectileEffect.LoadSynchronous();

    UNiagaraSystem* HitVFX = HitEffect.LoadSynchronous();

    USoundBase* HitSound = !HitSoundOverride.IsNull()
        ? HitSoundOverride.LoadSynchronous()
        : SkillData->SkillHitSound.LoadSynchronous();

    // 이 노티파이를 로컬에서 실행 중인 머신이 서버 권한을 가질 때만 실제 데미지 판정 수행
    const bool bApplyDamage = Owner->HasAuthority();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Owner->GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < ProjectileCount; i++)
    {
        // 투사체가 1개면 항상 정면으로, 여러 개면 SpreadAngleDegrees (범위) 안에서 균등 분산
        float AngleDeg = 0.f;
        if (ProjectileCount > 1)
        {
            float Alpha     = (float)i / (ProjectileCount - 1);
            float HalfAngle = SpreadAngleDegrees * 0.5f;
            AngleDeg = FMath::Lerp(-HalfAngle, HalfAngle, Alpha);
        }
        FVector Dir = ForwardDir.RotateAngleAxis(AngleDeg, FVector::UpVector);

        APTPlayerProjectileActor* Projectile = World->SpawnActor<APTPlayerProjectileActor>(
            APTPlayerProjectileActor::StaticClass(), SpawnPos, Dir.Rotation(), SpawnParams);

        if (!Projectile) continue;

        Projectile->InitProjectile(
            Owner,
            Dir,
            ProjectileSpeed,
            HitRadius,
            MaxRange,
            SkillData->DamageMultiplier,
            SkillData->bPenetrate,
            bApplyDamage,
            Effect,
            HitVFX,
            HitSound,
            SkillData->KnockbackForce,
            SkillData->KnockbackZForce,
            SkillData->HitStopDuration,
            SkillData->StaggerDuration,
            SkillData->HitReactionType,
            bDrawDebug
        );
    }
}
