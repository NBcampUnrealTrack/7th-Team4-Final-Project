#include "PTAnimNotify_ChannelActivate.h"

#include "Character/Player/PTPlayerCharacter.h"

void UPTAnimNotify_ChannelActivate::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(MeshComp->GetOwner()))
        PC->OnChannelSkillActivateNotify();
}
