#include "Character/Player/Anim/AN_AttackHit.h"

#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

void UAN_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!OwnerPlayer) return;

    FVector ForwardOffset = OwnerPlayer->GetActorForwardVector() * 150.f;
    FVector HeightOffset = FVector(0.f, 0.f, 50.f);
    FVector SphereCenter = OwnerPlayer->GetActorLocation() + ForwardOffset + HeightOffset;

    DrawDebugSphere(
    OwnerPlayer->GetWorld(),
    SphereCenter,
    150.f,          // SphereOverlapActors와 동일한 반경
    16,             // 세그먼트 수
    FColor::Red,
    false,          // 지속 여부
    1.f             // 표시 시간(초)
    );

    // 데미지는 서버에서만
    if (!OwnerPlayer->HasAuthority()) return;

    TArray<AActor*> HitActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        OwnerPlayer->GetWorld(),
        SphereCenter,
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
