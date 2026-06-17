
#include "Character/Player/Anim/PTAnimNotify_AttackHit.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

void UPTAnimNotify_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!OwnerPlayer) return;

    FVector BoxExtent = FVector(50.f, 100.f, 60.f);
    FVector BoxCenter = OwnerPlayer->GetActorLocation()
                      + OwnerPlayer->GetActorForwardVector() * 150.f
                      + FVector(0.f, 0.f, 50.f);
    FRotator BoxRotation = OwnerPlayer->GetActorRotation();

    DrawDebugBox(
        OwnerPlayer->GetWorld(),
        BoxCenter,
        BoxExtent,
        BoxRotation.Quaternion(),
        FColor::Red,
        false,
        0.5f
    );

    TArray<AActor*> HitActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::BoxOverlapActors(
        OwnerPlayer->GetWorld(),
        BoxCenter,
        BoxExtent,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{OwnerPlayer},
        HitActors
    );

    // 효과는 모든 클라이언트
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

    // 데미지는 서버에서
    if (!OwnerPlayer->HasAuthority()) return;

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
