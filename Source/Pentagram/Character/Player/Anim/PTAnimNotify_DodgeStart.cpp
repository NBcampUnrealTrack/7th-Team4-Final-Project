#include "PTAnimNotify_DodgeStart.h"

#include "Character/Player/PTPlayerCharacter.h"

void UPTAnimNotify_DodgeStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                      const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!PC) return;

    FVector DodgeDirection = PC->GetLastMovementInputVector();
    if (DodgeDirection.IsNearlyZero()) DodgeDirection = PC->GetActorForwardVector();

    PC->LaunchCharacter(DodgeDirection.GetSafeNormal2D() * DodgeDistance, true, false);

    PC->bIsInvincible = true;
}
