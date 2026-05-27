// Fill out your copyright notice in the Description page of Project Settings.


#include "PGInventoryComponent.h"

// Sets default values for this component's properties
UPGInventoryComponent::UPGInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UPGInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

    // 게임 시작 시 30칸의 빈 슬롯을 미리 확보. 
    InventorySlots.Init(FInventorySlot(), MaxSlotCount);
}

bool UPGInventoryComponent::TryAddItem(const FItemData& NewItemData, int32 Count)
{
    // 유효하지 않은 데이터나 수량 방어 코드
    if (NewItemData.Item_ID.IsNone() || Count <= 0) return false; 

    // 소비 아이템인 경우 기존에 같은 아이템이 있는지 먼저 확인 
    if (NewItemData.Item_Category == EItemCategory::Consumable)
    {
        int32 TargetIndex = FindSameItemSlot(NewItemData.Item_ID);
        if (TargetIndex != INDEX_NONE)
        {
            // 기존 슬롯을 찾았다면 수량만 증가 (스택 규칙 적용) 
            InventorySlots[TargetIndex].Quantity += Count;

            UE_LOG(LogTemp, Log, TEXT("[인벤토리] 기존 슬롯에 수량 추가: %s (+%d개, 총 %d개)"),
                *NewItemData.Item_Name.ToString(), Count, InventorySlots[TargetIndex].Quantity);

            PrintInventoryLog();
            return true;
        }
    }

    // 장비 아이템이거나 기존에 쌓인 소비 아이템 슬롯이 없다면 빈 슬롯을 탐색
    int32 EmptyIndex = FindEmptySlot();
    if (EmptyIndex != INDEX_NONE)
    {
        InventorySlots[EmptyIndex].ItemData = NewItemData;
        InventorySlots[EmptyIndex].Quantity = Count;

        UE_LOG(LogTemp, Log, TEXT("[인벤토리] 새 슬롯(%d번)에 아이템 등록: %s (%d개)"),
            EmptyIndex, *NewItemData.Item_Name.ToString(), Count);

        PrintInventoryLog();
        return true;
    }

    // 가방이 가득 차서 추가 실패
    UE_LOG(LogTemp, Warning, TEXT("[인벤토리] 가방 공간이 부족하여 아이템을 추가할 수 없습니다: %s"), *NewItemData.Item_Name.ToString());
    return false;
}

int32 UPGInventoryComponent::FindSameItemSlot(const FName& ItemID) const
{
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        // 빈 슬롯이 아니고, 카테고리가 소비창이며, 아이템 고유 ID가 일치하는지 검사
        if (!InventorySlots[i].IsEmpty() &&
            InventorySlots[i].ItemData.Item_Category == EItemCategory::Consumable &&
            InventorySlots[i].ItemData.Item_ID == ItemID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

int32 UPGInventoryComponent::FindEmptySlot() const
{
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].IsEmpty())
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void UPGInventoryComponent::PrintInventoryLog()
{
    UE_LOG(LogTemp, Log, TEXT("====== 현재 인벤토리 상태 (MVP) ======"));
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (!InventorySlots[i].IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("슬롯 [%d]: %s | 수량: %d | 타입: %d"),
                i,
                *InventorySlots[i].ItemData.Item_Name.ToString(),
                InventorySlots[i].Quantity,
                (int32)InventorySlots[i].ItemData.Item_Type);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("======================================"));
}
