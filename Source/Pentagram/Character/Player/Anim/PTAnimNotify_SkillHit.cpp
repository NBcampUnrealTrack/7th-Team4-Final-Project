#include "PTAnimNotify_SkillHit.h"

#include "NiagaraFunctionLibrary.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTSkillRow.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UPTAnimNotify_SkillHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                          const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!OwnerPlayer) return;

    UPTPlayerSkillComponent* SkillComp = OwnerPlayer->SkillComp;
    if (!SkillComp) return;

    FPTSkillRow* SkillData = SkillComp->GetSkillData(SkillComp->GetCurrentSkillID());
    if (!SkillData) return;

    FVector SphereCenter;
    switch (SkillData->IndicatorShape)
    {
    case ESkillIndicatorShape::Circle:
        // 커서로 조준한 지점 (컴포넌트에 캐시된 값)
        SphereCenter = SkillComp->TargetLocation;
        break;

    case ESkillIndicatorShape::SelfCircle:
        SphereCenter = OwnerPlayer->GetActorLocation();
        break;

    default: // Line / Cone 등 캐릭터 전방 기준
        {
            FVector Fwd   = OwnerPlayer->GetActorForwardVector() * SkillData->SkillOffset.X;
            FVector Right = OwnerPlayer->GetActorRightVector()   * SkillData->SkillOffset.Y;
            FVector Up    = FVector(0.f, 0.f, SkillData->SkillOffset.Z);
            SphereCenter  = OwnerPlayer->GetActorLocation() + Fwd + Right + Up;
        }
        break;
    }

    if (SkillData->TargetingMode == ESkillTargetingMode::Targeted)
    {
        AActor* T = SkillComp->TargetActor;
        APTBaseCharacter* Victim = Cast<APTBaseCharacter>(T);
        if (Victim && !Cast<APTPlayerCharacter>(Victim))
        {
            if (HitVFX)
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    OwnerPlayer->GetWorld(), HitVFX,
                    Victim->GetActorLocation() + FVector(0.f, 0.f, 50.f), FRotator::ZeroRotator);
            if (HitSFX)
                UGameplayStatics::PlaySoundAtLocation(
                    OwnerPlayer->GetWorld(), HitSFX, Victim->GetActorLocation());

            if (OwnerPlayer->HasAuthority())
            {
                FPTHitInfo HitInfo   = SkillData->MakeHitInfo(OwnerPlayer);
                HitInfo.HitDirection = (Victim->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal();
                Victim->ApplyDamageWithHit(OwnerPlayer->BaseAtk * SkillData->DamageMultiplier, OwnerPlayer, HitInfo);
            }
        }
        return;
    }

    TArray<AActor*> HitActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
    OwnerPlayer->GetWorld(),
    SphereCenter,
    SkillData->SkillRadius,
    ObjectTypes,
    nullptr,
    TArray<AActor*>{OwnerPlayer},
    HitActors
    );

    if (UNiagaraSystem* SkillVFX = SkillData->SkillEffect.LoadSynchronous())
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(OwnerPlayer->GetWorld(), SkillVFX, SphereCenter, FRotator::ZeroRotator);

    for (AActor* HitActor : HitActors)
    {
        if (APTBaseCharacter* Target = Cast<APTBaseCharacter>(HitActor))
        {
            if (Cast<APTPlayerCharacter>(Target)) continue;

            if (HitSFX)
                UGameplayStatics::PlaySoundAtLocation(
                    OwnerPlayer->GetWorld(),
                    HitSFX,
                    Target->GetActorLocation()
                );
        }
    }

    if (!OwnerPlayer->HasAuthority()) return;

    float FinalDamage = OwnerPlayer->BaseAtk * SkillData->DamageMultiplier;

    for (AActor* HitActor : HitActors)
    {
        if (APTBaseCharacter* Target = Cast<APTBaseCharacter>(HitActor))
        {
            if (Cast<APTPlayerCharacter>(Target)) continue;

            if (SkillData->IndicatorShape == ESkillIndicatorShape::Cone)
            {
                FVector ToTarget = (Target->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal2D();
                float Dot = FVector::DotProduct(OwnerPlayer->GetActorForwardVector(), ToTarget);
                float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(SkillData->IndicatorWidth * 0.5f));
                if (Dot < HalfAngleCos) continue;  // 부채꼴 밖이면 스킵
            }

            FVector HitDirection = (Target->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal();

            FPTHitInfo HitInfo   = SkillData->MakeHitInfo(OwnerPlayer);
            HitInfo.HitDirection = HitDirection;

            Target->ApplyDamageWithHit(FinalDamage, OwnerPlayer, HitInfo);
        }
    }
}
