#include "PTAnimNotifyState_IceSkillProjectile.h"

#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTSkillRow.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"

void UPTAnimNotifyState_IceSkillProjectile::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!Owner) return;

    UPTPlayerSkillComponent* SkillComp = Owner->SkillComp;
    FPTSkillRow* SkillData = SkillComp
        ? SkillComp->GetSkillData(SkillComp->GetCurrentSkillID())
        : nullptr;

    // 투사체 VFX - 모든 클라이언트에서 스폰
    if (SkillData)
    {
        if (UNiagaraSystem* Effect = SkillData->ProjectileEffect.LoadSynchronous())
        {
            FVector SpawnPos = Owner->GetActorLocation()
                + Owner->GetActorForwardVector() * SkillData->SkillOffset.X
                + Owner->GetActorRightVector()   * SkillData->SkillOffset.Y
                + FVector(0.f, 0.f,               SkillData->SkillOffset.Z);

            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                Owner->GetWorld(), Effect, SpawnPos, Owner->GetActorRotation());
        }
    }

    // 투사체 판정 초기화 -서버
    if (!Owner->HasAuthority()) return;

    FVector Offset = FVector::ZeroVector;
    if (SkillData)
    {
        Offset = Owner->GetActorForwardVector() * SkillData->SkillOffset.X
               + Owner->GetActorRightVector()   * SkillData->SkillOffset.Y
               + FVector(0.f, 0.f,               SkillData->SkillOffset.Z);
    }

    FVector StartPos   = Owner->GetActorLocation() + Offset;
    FVector ForwardDir = Owner->GetActorForwardVector();
    ForwardDir.Z = 0.f;
    ForwardDir.Normalize();

    Projectiles.Empty();

    for (int32 i = 0; i < ProjectileCount; i++)
    {
        float Alpha     = (ProjectileCount == 1) ? 0.f : (float)i / (ProjectileCount - 1);
        float HalfAngle = SpreadAngleDegrees * 0.5f;
        float AngleDeg  = FMath::Lerp(-HalfAngle, HalfAngle, Alpha);
        FVector Dir     = ForwardDir.RotateAngleAxis(AngleDeg, FVector::UpVector);

        FProjectileData Proj;
        Proj.CurrentPos = StartPos;
        Proj.Direction  = Dir;
        Proj.bExpired   = false;
        Projectiles.Add(Proj);
    }
}

void UPTAnimNotifyState_IceSkillProjectile::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!Owner || !Owner->HasAuthority()) return;

    UPTPlayerSkillComponent* SkillComp = Owner->SkillComp;
    FPTSkillRow* SkillData = SkillComp ? SkillComp->GetSkillData(SkillComp->GetCurrentSkillID()) : nullptr;
    if (!SkillData) return;

    FVector Origin = Owner->GetActorLocation();

    for (FProjectileData& Proj : Projectiles)
    {
        if (Proj.bExpired) continue;

        Proj.CurrentPos += Proj.Direction * ProjectileSpeed * FrameDeltaTime;

        if (FVector::Dist(Origin, Proj.CurrentPos) > MaxRange)
        {
            Proj.bExpired = true;
            continue;
        }

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

         float FinalDamage = Owner->GetTotalAttack() * SkillData->DamageMultiplier;

         FPTHitInfo HitInfo   = SkillData->MakeHitInfo(Owner);
         HitInfo.HitDirection = Proj.Direction;

         Target->ApplyDamageWithHit(FinalDamage, Owner, HitInfo);

         UE_LOG(LogTemp, Log, TEXT("Projectile %s 명중 / 데미지 %.1f"),
             *Target->GetName(), FinalDamage);

            if (USoundBase* HitSound = SkillData->SkillHitSound.LoadSynchronous())
            {
                Owner->SkillComp->Multicast_PlayHitSound(HitSound, Target->GetActorLocation());
            }

            if (!SkillData->bPenetrate) // DT 관통 여부
            {
                Proj.bExpired = true;
                break;
            }
     }
#if WITH_EDITOR
        if (bDrawDebug)
        {
            DrawDebugSphere(Owner->GetWorld(), Proj.CurrentPos,
                HitRadius, 8, FColor::Cyan, false, 0.1f);
        }
#endif
    }
}

void UPTAnimNotifyState_IceSkillProjectile::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    Projectiles.Empty(); // 메모리 정리
}
