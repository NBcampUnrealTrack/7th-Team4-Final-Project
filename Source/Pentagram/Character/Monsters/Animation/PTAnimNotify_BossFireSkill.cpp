#include "Character/Monsters/Animation/PTAnimNotify_BossFireSkill.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Character/Skill/PTBossPatternComponent.h"

void UPTAnimNotify_BossFireSkill::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!IsValid(MeshComp)) return;

    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Boss) || !Boss->HasAuthority()) return;

    UPTBossPatternComponent* PatternComp = Boss->GetBossPatternComponent();
    if (!IsValid(PatternComp)) return;

    PatternComp->ExecutePendingSkill();
}
