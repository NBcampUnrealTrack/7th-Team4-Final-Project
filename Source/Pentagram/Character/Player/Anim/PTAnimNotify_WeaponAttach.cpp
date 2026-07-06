#include "PTAnimNotify_WeaponAttach.h"

#include "Character/Player/PTPlayerCharacter.h"

void UPTAnimNotify_WeaponAttach::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* Character = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (Character)
    {
        Character->AttachWeaponToSocket(bAttachToHand);
    }
}

FString UPTAnimNotify_WeaponAttach::GetNotifyName_Implementation() const
{
    return bAttachToHand ? TEXT("WeaponToHand") : TEXT("WeaponToHolster");
}
