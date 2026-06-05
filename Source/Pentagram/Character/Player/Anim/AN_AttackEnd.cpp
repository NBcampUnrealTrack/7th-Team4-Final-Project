#include "Character/Player/Anim/AN_AttackEnd.h"

#include "Character/Player/PTPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAN_AttackEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(MeshComp->GetOwner()))
    {
        Player->bIsAttacking = false;
        Player->bCanCombo = false;
        Player->ComboIndex = 0;

        Player->GetCharacterMovement()->bOrientRotationToMovement = true;
        Player->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
    }
}
