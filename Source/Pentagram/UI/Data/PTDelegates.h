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

// 캐릭터 시트 - 전투 스탯 변경 (아이템 장착 시에도 이 델리게이트들로 반영 예정)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnAttackChanged, float, NewAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnDefenseChanged, float, NewDefense);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnCriticalChanged, float, CriticalChance, float, CriticalDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FPTOnMoveSpeedChanged, float, NewMoveSpeed);

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

//알림
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnNotifyFinished);

// 스킬 슬롯
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnSkillSlotAssigned, int32, SlotIndex, FName, SkillID);

// 로비 채팅
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnChatMessageReceived, FString, SenderName, FString, Message);
