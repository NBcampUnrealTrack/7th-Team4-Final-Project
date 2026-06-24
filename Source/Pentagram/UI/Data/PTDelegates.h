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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnBossSpawned, AActor*, BossActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnBossPhaseChanged, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnBossHealthChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnBossDefeated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnMonsterTargeted, AActor*, TargetMonster);

// 상점
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnShopBuyRequested, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnShopSlotHoverEvent, int32, SlotIndex);

//intro
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnIntroFinished);

//로비 준비
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnLobbyUpdated);

//장비
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipPanelRequested, int32, FromIndex, EItemType, EquipType);
