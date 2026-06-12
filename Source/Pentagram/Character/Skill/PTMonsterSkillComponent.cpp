#include "Character/Skill/PTMonsterSkillComponent.h"

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

    return GetCooldownRemaining(SlotIndex) <= 0.f;
}
