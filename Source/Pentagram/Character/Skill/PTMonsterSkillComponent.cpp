#include "Character/Skill/PTMonsterSkillComponent.h"
#include "Character/PTBaseCharacter.h"

bool UPTMonsterSkillComponent::PerformBasicAttack()
{
    if (!CanAttack())
    {
        return false;
    }

    FPTSkillActivationRequest Request;
    Request.SkillRowName   = BasicAttackRowName;
    Request.SkillDataTable = SkillDataTable;

    return TryActivateSkillChecked(Request);
}

bool UPTMonsterSkillComponent::CanAttack() const
{
    const int32 SlotIndex = SkillSlots.IndexOfByKey(BasicAttackRowName);
    if (SlotIndex == INDEX_NONE)
    {
        return false;
    }

    if (const APTBaseCharacter* Owner = Cast<APTBaseCharacter>(GetOwner()))
    {
        if (Owner->bIsStaggered)
        {
            return false;
        }
    }

    return GetCooldownRemaining(SlotIndex) <= 0.f;
}
