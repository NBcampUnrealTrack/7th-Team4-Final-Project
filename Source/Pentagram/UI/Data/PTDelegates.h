#pragma once

#include "CoreMinimal.h"
// TODO: 아래에 슬롯 데이터 구조체들이 정의된 헤더 파일을 인클루드 하세요.
// #include "PTSkillSlotData.h"
// #include "PTItemSlotData.h"

#include "PTDelegates.generated.h"


// 캐릭터 스탯

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnHealthChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnManaChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnExpChanged, float, Current, float, Required);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnGoldChanged, int64, NewAmount);

//스킬

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnSkillSlotChanged, int32, SlotIndex, FPTSkillSlotData, SlotData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnSkillCooldownUpdated, int32, SlotIndex, float, RemainingSeconds);

// 인벤토리/장비

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnInventoryChanged, const TArray<FPTItemSlotData>&, Slots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnEquipmentChanged);

// 전투(보스)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnBossSpawned, AActor*, BossActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnBossPhaseChanged, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnBossHealthChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnBossDefeated);
