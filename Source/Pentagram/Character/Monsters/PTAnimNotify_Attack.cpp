#include "Character/Monsters/PTAnimNotify_Attack.h"
#include "Character/Monsters/PTMonsterCharacter.h"

void UPTAnimNotify_Attack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation);

    if (!MeshComp)
    {
        return;
    }

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(MeshComp->GetOwner());
    if (!Monster)
    {
        return;
    }

    Monster->PerformAttack();
}
