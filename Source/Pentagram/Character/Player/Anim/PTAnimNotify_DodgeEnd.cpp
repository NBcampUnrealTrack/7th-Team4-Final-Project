#include "PTAnimNotify_DodgeEnd.h"

#include "Character/Player/PTPlayerCharacter.h"

void UPTAnimNotify_DodgeEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!PC) return;

    PC->bIsInvincible = false;
}
