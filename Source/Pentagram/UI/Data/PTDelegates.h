#pragma once

#include "CoreMinimal.h"
#include "PTDelegates.generated.h"

USTRUCT(BlueprintType)
struct FPTDelegateDummy
{
    GENERATED_BODY()
};

// 매개변수 이름이 내부 매크로와 충돌하지 않도록 변경
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnHealthChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnManaChanged, float, CurrentMP, float, MaxMP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnExpChanged, float, CurrentExp, float, RequiredExp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnGoldChanged, int64, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnSkillCooldownStart, int32, SlotIndex, float, Duration);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTMonsterHealthChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnBossSpawned, AActor*, BossActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnBossPhaseChanged, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnBossHealthChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnBossDefeated);
