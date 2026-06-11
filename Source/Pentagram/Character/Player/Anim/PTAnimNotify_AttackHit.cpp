#include "Character/Player/Anim/PTAnimNotify_AttackHit.h"

#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

void UPTAnimNotify_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

    /* PTBaseCharacter::ApplyDamage 내부에서 따로 공격자(OwnerPlayer)의 장비창 스탯을 알아서 더해주기 때문에
    복잡한 장비 스탯 계산 없이 기본 공격력만 넘겨줌 */ 
    for (AActor* HitActor : HitActors)
    {
        if (HitActor && HitActor != OwnerPlayer)
        {
            if (APTBaseCharacter* BaseChar = Cast<APTBaseCharacter>(HitActor))
            {
                // 부모의 ApplyDamage로 기본 공격력을 넘기면, 장비 보너스가 자동 합산됨
                BaseChar->ApplyDamage(OwnerPlayer->BaseAtk, OwnerPlayer);
            }
        }
    }
}
