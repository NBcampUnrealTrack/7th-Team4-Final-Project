#include "PTEquipmentComponent.h"

#include "Net/UnrealNetwork.h"

UPTEquipmentComponent::UPTEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // 네트워크 리플리케이트 활성화
    SetIsReplicatedByDefault(true);

    // 슬롯 초기화
    EquippedWeapon = FEquipmentSlot(EEquipSlotType::Weapon);
    EquippedChest  = FEquipmentSlot(EEquipSlotType::Chest);

    TotalBonusStr = 0;
    TotalBonusDef = 0;
    TotalBonusHp  = 0;
}

void UPTEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
}

// 동기화할 데이터(슬롯 및 스탯들)를 네트워크 매크로에 등록
void UPTEquipmentComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 복제 규칙 등록 = 슬롯
    DOREPLIFETIME(UPTEquipmentComponent, EquippedWeapon);
    DOREPLIFETIME(UPTEquipmentComponent, EquippedChest);

    // 복제 규칙 등록 = 합산 보너스 스탯
    DOREPLIFETIME(UPTEquipmentComponent, TotalBonusStr);
    DOREPLIFETIME(UPTEquipmentComponent, TotalBonusDef);
    DOREPLIFETIME(UPTEquipmentComponent, TotalBonusHp);
}

bool UPTEquipmentComponent::EquipItem(const FItemData& NewItem, FItemData& OutOldItem) // 장착 로직
{
    // 장비 카테고리가 아니면 예외 처리
    if (NewItem.Item_Category != EItemCategory::Equipment) return false;

    // [멀티플레이어 분기] 클라이언트가 장착을 시도한 경우
    if (!GetOwner()->HasAuthority())
    {
        // 서버에게 장착 요청 무전(RPC)을 보냄.
        Server_EquipItem(NewItem);
        OutOldItem = FItemData(); // 클라이언트는 즉시 반환값을 예측할 수 없으므로 공데이터 리턴
        return true;
    }

    // 여기서부터 오직 '서버' 권한으로만 실행되는 구역
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

    // 실시간 보너스 스탯 누적 및 캐릭터 갱신 (리플리케이트로 인해 클라이언트로 전송될 것)
    UpdateTotalBonusStats();

    UE_LOG(LogTemp, Log, TEXT("[장비컴포넌트] 장착 완료: %s (누적 스탯 -> STR: %d, DEF: %d, HP: %d)"),
        *NewItem.Item_Name.ToString(), TotalBonusStr, TotalBonusDef, TotalBonusHp);

    return true;
}

bool UPTEquipmentComponent::UnequipItem(EEquipSlotType SlotType, FItemData& OutUnequippedItem) // 해제 로직
{
    // [멀티플레이어 분기] 클라이언트가 장비 해제를 시도한 경우
    if (!GetOwner()->HasAuthority())
    {
        // 서버에게 해제 요청 무전(RPC)을 보냄.
        Server_UnequipItem(SlotType);
        OutUnequippedItem = FItemData();
        return true;
    }

    // 여기서부터 오직 '서버' 권한으로만 실행되는 구역
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

// 장착 Server RPC 구현부
void UPTEquipmentComponent::Server_EquipItem_Implementation(const FItemData& NewItem)
{
    FItemData DummyOldItem;
    EquipItem(NewItem, DummyOldItem);

    /* 테스트 중 데이터 꼬임없이 서버와 클라이언트 간에 장착/해제 및 스탯계산이 완벽하다면
    (장비 교체 시) 원래 장착중인 장비를 인벤토리에 다시 넣어주는 연동 처리를 나중에 여기에 구현하기. */
}

bool UPTEquipmentComponent::Server_EquipItem_Validate(const FItemData& NewItem)
{
    // 잘못된 아이템 데이터 패킷 필터링
    if (NewItem.Item_ID.IsNone()) return false;
    return true;
}

// 해제 Server RPC 구현부
void UPTEquipmentComponent::Server_UnequipItem_Implementation(EEquipSlotType SlotType)
{
    FItemData DummyUnequippedItem;
    UnequipItem(SlotType, DummyUnequippedItem);

    /* 테스트 중 데이터 꼬임없이 서버와 클라이언트 간에 장착/해제 및 스탯계산이 완벽하다면
    해제된 장비를 인벤토리에 다시 넣어주는 연동 처리를 나중에 여기에 구현하기. */
}

bool UPTEquipmentComponent::Server_UnequipItem_Validate(EEquipSlotType SlotType)
{
    return true;
}

void UPTEquipmentComponent::UpdateTotalBonusStats()
{
    // 스탯 초기화 후 처음부터 다시 계산 (데이터 동기화 안정성/버그방지 확보)
    TotalBonusStr = 0;
    TotalBonusDef = 0;
    TotalBonusHp  = 0;

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
                    if (Option.Contains(TEXT("STR+5")))   TotalBonusStr += 5;
                    if (Option.Contains(TEXT("DEF+5")))   TotalBonusDef += 5;
                    if (Option.Contains(TEXT("MaxHP+20"))) TotalBonusHp += 20;
                }
            }
        }
    }
}
