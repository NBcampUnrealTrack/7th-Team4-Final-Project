
#include "PGEquipmentComponent.h"

// Sets default values for this component's properties
UPGEquipmentComponent::UPGEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

    // 슬롯 초기화 
    EquippedWeapon = FEquipmentSlot(EEquipSlotType::Weapon);
    EquippedChest = FEquipmentSlot(EEquipSlotType::Chest);

    TotalBonusStr = 0;
    TotalBonusDef = 0;
    TotalBonusHp = 0;
}



void UPGEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

}


bool UPGEquipmentComponent::EquipItem(const FItemData& NewItem, FItemData& OutOldItem) // 장착 로직 
{
    // 장비 카테고리가 아니면 예외 처리
    if (NewItem.Item_Category != EItemCategory::Equipment) return false;

    FEquipmentSlot* TargetSlot = nullptr;

    // 아이템 세부 타입에 따라 대상 슬롯 지정
    if (NewItem.Item_Type == EItemType::Weapon)
    {
        TargetSlot = &EquippedWeapon;
    }
    else if (NewItem.Item_Type == EItemType::Chest)
    {
        TargetSlot = &EquippedChest;
    }

    if (!TargetSlot) return false;

    // 이미 장착 중이라면 기존 장비를 꺼내서 반환용 변수에 저장
    if (TargetSlot->bIsEquipped)
    {
        OutOldItem = TargetSlot->MountedItem;
    }
    else
    {
        // 비어있었다면 공 데이터 처리
        OutOldItem = FItemData();
    }

    // 새 장비 장착 데이터 처리
    TargetSlot->MountedItem = NewItem;
    TargetSlot->bIsEquipped = true;

    // 실시간 보너스 스탯 누적 및 캐릭터 갱신
    UpdateTotalBonusStats();

    UE_LOG(LogTemp, Log, TEXT("[장비컴포넌트] 장착 완료: %s (누적 스탯 -> STR: %d, DEF: %d, HP: %d)"),
        *NewItem.Item_Name.ToString(), TotalBonusStr, TotalBonusDef, TotalBonusHp);

    return true;
}

bool UPGEquipmentComponent::UnequipItem(EEquipSlotType SlotType, FItemData& OutUnequippedItem) // 해제 로직
{
    FEquipmentSlot* TargetSlot = (SlotType == EEquipSlotType::Weapon) ? &EquippedWeapon : &EquippedChest;

    if (!TargetSlot || !TargetSlot->bIsEquipped) return false;

    // 데이터 해제 처리 흐름
    OutUnequippedItem = TargetSlot->MountedItem;
    TargetSlot->bIsEquipped = false;
    TargetSlot->MountedItem = FItemData();

    // 스탯 차감 반영
    UpdateTotalBonusStats();

    UE_LOG(LogTemp, Log, TEXT("[장비컴포넌트] 해제 완료: %s (누적 스탯 -> STR: %d, DEF: %d, HP: %d)"),
        *OutUnequippedItem.Item_Name.ToString(), TotalBonusStr, TotalBonusDef, TotalBonusHp);

    return true;
}

void UPGEquipmentComponent::UpdateTotalBonusStats()
{
    // 스탯 초기화 후 처음부터 다시 계산 (데이터 동기화 안정성/버그방지 확보)
    TotalBonusStr = 0;
    TotalBonusDef = 0;
    TotalBonusHp = 0;

    TArray<FEquipmentSlot*> Slots = { &EquippedWeapon, &EquippedChest };

    for (FEquipmentSlot* Slot : Slots)
    {
        if (Slot && Slot->bIsEquipped)
        {
            // 1. 기본 성능 파싱 (무기: STR, 갑옷: DEF)
            if (Slot->EquippedSlotType == EEquipSlotType::Weapon)
            {
                TotalBonusStr += Slot->MountedItem.Item_Base_Stat;
            }
            else if (Slot->EquippedSlotType == EEquipSlotType::Chest)
            {
                TotalBonusDef += Slot->MountedItem.Item_Base_Stat;
            }

            // 2. Rare 등급 추가 랜덤 옵션 파싱 (간단한 문자열 매칭 검증)
            if (Slot->MountedItem.Item_Grade == EItemGrade::Rare)
            {
                for (const FString& Option : Slot->MountedItem.Item_Bonus_Options)
                {
                    if (Option.Contains(TEXT("STR+5"))) TotalBonusStr += 5;
                    if (Option.Contains(TEXT("DEF+5"))) TotalBonusDef += 5;
                    if (Option.Contains(TEXT("MaxHP+20"))) TotalBonusHp += 20;
                }
            }
        }
    }
}

