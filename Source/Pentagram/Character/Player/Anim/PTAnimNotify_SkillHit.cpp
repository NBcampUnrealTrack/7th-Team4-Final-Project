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

    FVector ForwardOffset = OwnerPlayer->GetActorForwardVector() * SkillData->SkillOffset.X;
    FVector RightOffset = OwnerPlayer->GetActorRightVector() * SkillData->SkillOffset.Y;
    FVector HeightOffset = FVector(0.f,0.f, SkillData->SkillOffset.Z);
    FVector SphereCenter = OwnerPlayer->GetActorLocation() + ForwardOffset + RightOffset + HeightOffset;

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

    for (AActor* HitActor : HitActors)
    {
        if (APTBaseCharacter* Target = Cast<APTBaseCharacter>(HitActor))
        {
            if (Cast<APTPlayerCharacter>(Target)) continue;

            if (HitVFX)
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    OwnerPlayer->GetWorld(),
                    HitVFX,
                    Target->GetActorLocation() + FVector(0.f, 0.f, 50.f),
                    FRotator::ZeroRotator
                );

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
            Target->ApplyDamage(FinalDamage, OwnerPlayer);
        }
    }
}
