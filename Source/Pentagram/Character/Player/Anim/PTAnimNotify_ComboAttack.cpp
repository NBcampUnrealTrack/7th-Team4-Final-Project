#include "Character/Player/Anim/PTAnimNotify_ComboAttack.h"

#include "Character/Player/PTPlayerCharacter.h"


void UPTAnimNotify_ComboAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner()))
    {
        OwnerPlayer->bCanCombo = true;
    }
}

void UPTAnimNotify_ComboAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner()))
    {
        OwnerPlayer->bCanCombo = false;
    }
}
