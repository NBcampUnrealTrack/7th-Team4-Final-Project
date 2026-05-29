#include "Character/Player/Anim/AN_AttackHit.h"

#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

void UAN_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!OwnerPlayer) return;
    if (!OwnerPlayer->HasAuthority()) return;

    TArray<AActor*> HitActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        OwnerPlayer->GetWorld(),
        OwnerPlayer->GetActorLocation(),
        150.f,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{OwnerPlayer},
        HitActors
    );

    for (AActor* HitActor : HitActors)
    {
        if (APTBaseCharacter* Target = Cast<APTBaseCharacter>(HitActor))
        {
            Target->ApplyDamage(OwnerPlayer->BaseAtk, OwnerPlayer);
        }
    }
}
