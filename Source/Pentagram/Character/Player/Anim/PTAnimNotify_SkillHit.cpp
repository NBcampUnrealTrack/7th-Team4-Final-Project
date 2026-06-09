#include "PTAnimNotify_SkillHit.h"

#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTSkillRow.h"
#include "Character/Skill/PTSkillComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void UPTAnimNotify_SkillHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                          const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!OwnerPlayer || !OwnerPlayer->HasAuthority()) return;

    UPTSkillComponent* SkillComp = OwnerPlayer->SkillComp;
    if (!SkillComp) return;

    FPTSkillRow* SkillData = SkillComp->GetSkillData(SkillComp->CurrentSkillID);
    if (!SkillData) return;

    float FinalDamage = OwnerPlayer->BaseAtk * SkillData->DamageMultiplier;

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
            Target->ApplyDamage(FinalDamage, OwnerPlayer);
        }
    }
}
