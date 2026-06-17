#include "Character/Monsters/Animation/PTAnimNotify_BossFireSkill.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Character/Skill/PTBossPatternComponent.h"

void UPTAnimNotify_BossFireSkill::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!IsValid(MeshComp))
    {
        return;
    }

    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Boss))
    {
        return;
    }

    if (UPTBossPatternComponent* PatternComp = Boss->GetBossPatternComponent())
    {
        PatternComp->ExecutePendingSkill(Boss->GetCurrentPhase());
    }
}
