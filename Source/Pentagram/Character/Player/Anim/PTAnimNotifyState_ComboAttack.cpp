#include "Character/Player/Anim/PTAnimNotifyState_ComboAttack.h"

#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTPlayerSkillComponent.h"

void UPTAnimNotifyState_ComboAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner()))
    {
        if (OwnerPlayer->SkillComp)
        {
            OwnerPlayer->SkillComp->bCanCombo = true;
        }
    }
}

void UPTAnimNotifyState_ComboAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner()))
    {
        if (OwnerPlayer->SkillComp)
        {
            OwnerPlayer->SkillComp->bCanCombo = false;
        }
    }
}
