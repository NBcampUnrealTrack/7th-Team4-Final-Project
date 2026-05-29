#include "Character/Player/Anim/AN_AttackEnd.h"

#include "Character/Player/PTPlayerCharacter.h"

void UAN_AttackEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(MeshComp->GetOwner()))
    {
        Player->bIsAttacking = false;
        Player->bCanCombo = false;
        Player->ComboIndex = 0;
    }
}
