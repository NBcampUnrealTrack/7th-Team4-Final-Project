#include "PTAnimNotifyState_SkillProjectile.h"

#include "Character/Player/PTPlayerCharacter.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Skill/PTSkillRow.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"

void UPTAnimNotifyState_SkillProjectile::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!Owner) return;

    UPTPlayerSkillComponent* SkillComp = Owner->SkillComp;

    // CurrentSkillID가 아니라, 이 노티파이에 지정된 SkillRowName으로 직접 조회
    FPTSkillRow* SkillData = SkillComp
        ? SkillComp->GetSkillData(SkillRowName)
        : nullptr;

    if (!SkillData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTAnimNotifyState_Projectile] SkillRowName '%s'에 해당하는 DT 로우를 찾을 수 없음"),
            *SkillRowName.ToString());
        return;
    }

    FVector Offset = Owner->GetActorForwardVector() * SkillData->SkillOffset.X
                    + Owner->GetActorRightVector() * SkillData->SkillOffset.Y
                    + FVector(0.f, 0.f, SkillData->SkillOffset.Z);

    FVector StartPos   = Owner->GetActorLocation() + Offset;
    FVector ForwardDir = Owner->GetActorForwardVector();
    ForwardDir.Z = 0.f;
    ForwardDir.Normalize();

    // 투사체를 따라다닐 나이아가라 이펙트
    UNiagaraSystem* Effect = !ProjectileEffectOverride.IsNull()
    ? ProjectileEffectOverride.LoadSynchronous()
    : SkillData->ProjectileEffect.LoadSynchronous();

    Projectiles.Empty();

    for (int32 i = 0; i < ProjectileCount; i++)
    {
        float Alpha     = (ProjectileCount == 1) ? 0.f : (float)i / (ProjectileCount - 1);
        float HalfAngle = SpreadAngleDegrees * 0.5f;
        float AngleDeg  = FMath::Lerp(-HalfAngle, HalfAngle, Alpha);
        FVector Dir     = ForwardDir.RotateAngleAxis(AngleDeg, FVector::UpVector);

        FProjectileRuntimeData Proj;
        Proj.CurrentPos = StartPos;
        Proj.Direction  = Dir;
        Proj.bExpired   = false;

        // 투사체 이동을 따라가는 VFX
        if (Effect)
        {
            Proj.TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                Owner->GetWorld(),
                Effect,
                Proj.CurrentPos,
                Dir.Rotation(),
                FVector(1.f),
                true,
                true,
                ENCPoolMethod::None,
                true
            );
        }

        Projectiles.Add(Proj);
    }

    // 데미지 초기화는 서버
    if (!Owner->HasAuthority()) return;
}

void UPTAnimNotifyState_SkillProjectile::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!Owner) return;

    UPTPlayerSkillComponent* SkillComp = Owner->SkillComp;
    FPTSkillRow* SkillData = SkillComp ? SkillComp->GetSkillData(SkillRowName) : nullptr;
    if (!SkillData) return;

    FVector Origin = Owner->GetActorLocation();
    const bool bAuthority = Owner->HasAuthority();

    for (FProjectileRuntimeData& Proj : Projectiles)
    {
        if (Proj.bExpired) continue;

        Proj.CurrentPos += Proj.Direction * ProjectileSpeed * FrameDeltaTime;

        if (Proj.TrailComponent)
        {
            Proj.TrailComponent->SetWorldLocation(Proj.CurrentPos);
        }

        if (FVector::Dist(Origin, Proj.CurrentPos) > MaxRange)
        {
            ExpireProjectile(Proj);
            continue;
        }

#if WITH_EDITOR
        if (bDrawDebug)
        {
            DrawDebugSphere(Owner->GetWorld(), Proj.CurrentPos,
                HitRadius, 8, FColor::Cyan, false, 0.1f);
        }
#endif

        if (!bAuthority) continue;

        TArray<AActor*> OverlapActors;
        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
        ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

        UKismetSystemLibrary::SphereOverlapActors(
            Owner->GetWorld(),
            Proj.CurrentPos,
            HitRadius,
            ObjectTypes,
            nullptr,
            TArray<AActor*>{ Owner },
            OverlapActors
        );

        for (AActor* HitActor : OverlapActors)
        {
            if (Proj.HitActors.Contains(TWeakObjectPtr<AActor>(HitActor))) continue;

            APTBaseCharacter* Target = Cast<APTBaseCharacter>(HitActor);
            if (!Target || Cast<APTPlayerCharacter>(Target)) continue;

            Proj.HitActors.Add(TWeakObjectPtr<AActor>(HitActor));

            const float FinalDamage = Owner->GetTotalAttack() * SkillData->DamageMultiplier;

            FPTHitInfo HitInfo   = SkillData->MakeHitInfo(Owner);
            HitInfo.HitDirection = Proj.Direction;

            Target->ApplyDamageWithHit(FinalDamage, Owner, HitInfo);

            UE_LOG(LogTemp, Log, TEXT("Projectile %s 명중 / 데미지 %.1f"),
                *Target->GetName(), FinalDamage);

            if (USoundBase* SoundToPlay = SkillData->SkillHitSound.LoadSynchronous())
            {
                Owner->SkillComp->Multicast_PlayHitSound(SoundToPlay, Target->GetActorLocation());
            }

            if (!SkillData->bPenetrate)
            {
                ExpireProjectile(Proj);
                break;
            }
        }
    }
}

void UPTAnimNotifyState_SkillProjectile::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    for (FProjectileRuntimeData& Proj : Projectiles)
    {
        if (Proj.TrailComponent)
        {
            Proj.TrailComponent->Deactivate();
            Proj.TrailComponent = nullptr;
        }
    }

    Projectiles.Empty();
}

void UPTAnimNotifyState_SkillProjectile::ExpireProjectile(FProjectileRuntimeData& Proj)
{
    Proj.bExpired = true;

    if (Proj.TrailComponent)
    {
        Proj.TrailComponent->Deactivate();
        Proj.TrailComponent = nullptr;
    }
}
