#include "Character/Monsters/Animation/PTAnimNotify_Attack.h"
#include "Character/Monsters/PTMonsterCharacter.h"

void UPTAnimNotify_Attack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!IsValid(MeshComp))
    {
        return;
    }

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Monster))
    {
        return;
    }

    Monster->PerformAttack();
}
