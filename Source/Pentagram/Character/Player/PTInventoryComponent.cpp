// Fill out your copyright notice in the Description page of Project Settings.


#include "PTInventoryComponent.h"
#include "Character/PTBaseCharacter.h" // 캐릭터 내부 HP 변수에 접근


// Sets default values for this component's properties
UPTInventoryComponent::UPTInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UPTInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

    // 게임 시작 시 30칸의 빈 슬롯을 미리 확보. 
    InventorySlots.Init(FInventorySlot(), MaxSlotCount); 
}

bool UPTInventoryComponent::TryAddItem(const FItemData& NewItemData, int32 Count)
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

int32 UPTInventoryComponent::FindSameItemSlot(const FName& ItemID) const
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

int32 UPTInventoryComponent::FindEmptySlot() const
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

void UPTInventoryComponent::PrintInventoryLog()
{
    UE_LOG(LogTemp, Log, TEXT("========= 현재 인벤토리 상태 ========="));
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

bool UPTInventoryComponent::UsePotion(int32 SlotIndex) // 소모아이템(포션) 사용 
{
    // 슬롯 인덱스 유효성 검사 및 빈 슬롯 검사
    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty()) return false;

    // 카테고리가 포션(Consumable)이 맞는지 확인
    if (InventorySlots[SlotIndex].ItemData.Item_Category != EItemCategory::Consumable)
    {
        UE_LOG(LogTemp, Warning, TEXT("소비 아이템이 아닙니다."));
        return false;
    } 

    // 소비 아이템 개수 차감 
    InventorySlots[SlotIndex].Quantity--;
    UE_LOG(LogTemp, Log, TEXT("%s 아이템 사용. 남은 수량: %d개"), 
        *InventorySlots[SlotIndex].ItemData.Item_Name.ToString(), InventorySlots[SlotIndex].Quantity);

    // 가방 상황 로그 출력 
    PrintInventoryLog(); 

    // 5초 동안 매초 서서히 회복되는 타이머 가동 
    GetWorld()->GetTimerManager().ClearTimer(PotionTimerHandle); // 기존에 돌던 포션 타이머가 있다면 초기화 
    PotionTickCount = 0; // 틱 카운터 초기화 

    // 1초마다 ExecutePotionHealing 함수를 반복 호출 (총 5회)
    GetWorld()->GetTimerManager().SetTimer(PotionTimerHandle, this, &UPTInventoryComponent::ExecutePotionHealing, 1.0f, true);

    return true;
}


void UPTInventoryComponent::ExecutePotionHealing() // 포션 회복  
{
    // 이 컴포넌트를 들고 있는 주인(캐릭터) 가져오기
    APTBaseCharacter* OwnerCharacter = Cast<APTBaseCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        GetWorld()->GetTimerManager().ClearTimer(PotionTimerHandle);
        return;
    }

    PotionTickCount++;

    // 매초 전체 HP의 5%씩 회복 
    float HealAmount = OwnerCharacter->MaxHP * 0.05f;

    // 현재 체력이 최대 체력을 넘지 않도록 회복 
    OwnerCharacter->CurrentHP = FMath::Min(OwnerCharacter->CurrentHP + HealAmount, OwnerCharacter->MaxHP);

    UE_LOG(LogTemp, Log, TEXT("[포션 틱 %d회차] 5%% 회복 (+%.1f) -> 현재 HP: %.1f / %.1f"),
        PotionTickCount, HealAmount, OwnerCharacter->CurrentHP, OwnerCharacter->MaxHP);

    // 5초(5번 틱)가 지나면 타이머 종료 
    if (PotionTickCount >= 5)
    {
        GetWorld()->GetTimerManager().ClearTimer(PotionTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("[포션 효과 끝남]"));
    }
}
