#include "PTEquipmentComponent.h"
#include "Net/UnrealNetwork.h"
#include "PTPlayerCharacter.h"


UPTEquipmentComponent::UPTEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    EquippedWeapon = FEquipmentSlot(EEquipSlotType::Weapon);
    EquippedChest  = FEquipmentSlot(EEquipSlotType::Chest);
    EquippedHelmet = FEquipmentSlot(EEquipSlotType::Helmet);
    EquippedGloves = FEquipmentSlot(EEquipSlotType::Gloves);
    EquippedBoots = FEquipmentSlot(EEquipSlotType::Boots);

    TotalBonusStr = 0;
    TotalBonusDef = 0;
    TotalBonusHp  = 0;
}

bool UPTEquipmentComponent::IsWeaponEquipped() const
{
    return EquippedWeapon.bIsEquipped;
}

void UPTEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
}

// 리플리케이션 규칙
void UPTEquipmentComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPTEquipmentComponent, EquippedWeapon);
    DOREPLIFETIME(UPTEquipmentComponent, EquippedChest);
    DOREPLIFETIME(UPTEquipmentComponent, EquippedHelmet);
    DOREPLIFETIME(UPTEquipmentComponent, EquippedGloves);
    DOREPLIFETIME(UPTEquipmentComponent, EquippedBoots);

    DOREPLIFETIME(UPTEquipmentComponent, TotalBonusStr);
    DOREPLIFETIME(UPTEquipmentComponent, TotalBonusDef);
    DOREPLIFETIME(UPTEquipmentComponent, TotalBonusHp);
}

bool UPTEquipmentComponent::EquipItem(const FItemData& NewItem, FItemData& OutOldItem) // 장착
{
    if (NewItem.Item_Category != EItemCategory::Equipment) return false; // 장비 카테고리가 맞나

    if (!GetOwner()->HasAuthority())
    {
        // 서버에게 장착 요청 RPC
        Server_EquipItem(NewItem);
        OutOldItem = FItemData();
        return true;
    }

    // 여기서부터 오직 '서버' 권한으로만 실행
    FEquipmentSlot* TargetSlot = nullptr;


    // 아이템 세부 타입에 따라 대상 슬롯 지정
    if (NewItem.Item_Type == EItemType::Weapon)     TargetSlot = &EquippedWeapon;
    else if (NewItem.Item_Type == EItemType::Chest)  TargetSlot = &EquippedChest;
    else if (NewItem.Item_Type == EItemType::Helmet) TargetSlot = &EquippedHelmet;
    else if (NewItem.Item_Type == EItemType::Gloves) TargetSlot = &EquippedGloves;
    else if (NewItem.Item_Type == EItemType::Boots)  TargetSlot = &EquippedBoots;
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

    // 새 장비 장착 데이터 처리 (구조체 리플리케이션 안전 동기화를 위한 통대입 처리)
    FEquipmentSlot UpdatedSlot = *TargetSlot;
    UpdatedSlot.MountedItem = NewItem;
    UpdatedSlot.bIsEquipped = true;

    // 구조체 자체를 대입하여 RPC/네트워크 복제 유도
    *TargetSlot = UpdatedSlot;

    // 실시간 보너스 스탯 누적 및 캐릭터 갱신 (리플리케이트로 인해 클라이언트로 전송될 것)
    UpdateTotalBonusStats();

    // 장착한 아이템이 무기일 때, 캐릭터의 외형 메시를 업데이트
    if (NewItem.Item_Type == EItemType::Weapon)
    {
        // 이 컴포넌트의 주인(Owner)을 플레이어 캐릭터로 캐스팅
        APTPlayerCharacter* OwnerCharacter = Cast<APTPlayerCharacter>(GetOwner());
        if (OwnerCharacter)
        {
            // 데이터베이스에서 세팅한 메시를 캐릭터에게 전달
            OwnerCharacter->UpdateWeaponVisual(NewItem.ItemMeshAsset, NewItem);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[장비컴포넌트] 장착 완료: %s (누적 스탯 -> STR: %d, DEF: %d, HP: %d)"),
        *NewItem.Item_Name.ToString(), TotalBonusStr, TotalBonusDef, TotalBonusHp);

    return true;
}

bool UPTEquipmentComponent::UnequipItem(EEquipSlotType SlotType, FItemData& OutUnequippedItem) // 장비 해제
{
    if (!GetOwner()->HasAuthority())
    {
        // 서버에게 해제 요청 RPC
        Server_UnequipItem(SlotType);
        OutUnequippedItem = FItemData();
        return true;
    }

    // 여기서부터 오직 '서버' 권한으로만 실행
    FEquipmentSlot* TargetSlot = nullptr;
    switch (SlotType)
    {
        case EEquipSlotType::Weapon: TargetSlot = &EquippedWeapon; break;
        case EEquipSlotType::Chest:  TargetSlot = &EquippedChest;  break;
        case EEquipSlotType::Helmet: TargetSlot = &EquippedHelmet; break;
        case EEquipSlotType::Gloves: TargetSlot = &EquippedGloves; break;
        case EEquipSlotType::Boots:  TargetSlot = &EquippedBoots;  break;
    }

    if (!TargetSlot || !TargetSlot->bIsEquipped) return false;

    // 데이터 해제 처리 흐름
    OutUnequippedItem = TargetSlot->MountedItem;

    FEquipmentSlot ClearedSlot = FEquipmentSlot(SlotType);
    ClearedSlot.bIsEquipped = false;
    ClearedSlot.MountedItem = FItemData();

    *TargetSlot = ClearedSlot;

    UpdateTotalBonusStats(); // 스탯 차감 반영

    // 해제한 슬롯이 무기 슬롯일 때, 캐릭터의 외형 메시를 비움
    if (SlotType == EEquipSlotType::Weapon)
    {
        APTPlayerCharacter* OwnerCharacter = Cast<APTPlayerCharacter>(GetOwner());
        if (OwnerCharacter)
        {
            // nullptr이나 IsNull() 상태라면 메시를 비움
            OwnerCharacter->UpdateWeaponVisual(TSoftObjectPtr<UStaticMesh>());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[장비컴포넌트] 해제 완료: %s (누적 스탯 -> STR: %d, DEF: %d, HP: %d)"),
        *OutUnequippedItem.Item_Name.ToString(), TotalBonusStr, TotalBonusDef, TotalBonusHp);

    return true;
}

// 장착 Server RPC 구현부
void UPTEquipmentComponent::Server_EquipItem_Implementation(const FItemData& NewItem)
{
    FItemData DummyOldItem;
    EquipItem(NewItem, DummyOldItem);

    /* TODO : [테스트 중 데이터 꼬임없이 서버와 클라이언트 간에 장착/해제 및 스탯계산이 완벽하다면
    (장비 교체 시) 원래 장착중인 장비의 ID가 None이 아니라면 인벤토리에 다시 넣어주는 연동 처리 구현하기] */
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

    /* TODO : [테스트 중 데이터 꼬임없이 서버와 클라이언트 간에 장착/해제 및 스탯계산이 완벽하다면
    해제된 장비를 인벤토리에 다시 넣어주는 연동 처리를 나중에 여기에 구현하기] */
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

    TArray<FEquipmentSlot*> Slots = { &EquippedWeapon, &EquippedChest, &EquippedHelmet, &EquippedGloves, &EquippedBoots };

    for (FEquipmentSlot* Slot : Slots)
    {
        if (Slot && Slot->bIsEquipped) // 기본 옵션 파싱
        {
            // (무기/장갑: STR)
            if (Slot->EquippedSlotType == EEquipSlotType::Weapon || Slot->EquippedSlotType == EEquipSlotType::Gloves)
            {
                TotalBonusStr += Slot->MountedItem.Item_Base_Stat;
            }
            // (갑옷/신발: DEF)
            else if (Slot->EquippedSlotType == EEquipSlotType::Chest || Slot->EquippedSlotType == EEquipSlotType::Boots)
            {
                TotalBonusDef += Slot->MountedItem.Item_Base_Stat;
            }
            // (모자: MaxHP)
            else if (Slot->EquippedSlotType == EEquipSlotType::Helmet)
            {
                TotalBonusHp += Slot->MountedItem.Item_Base_Stat;
            }

            // 등급 추가 옵션 파싱 (간단한 문자열 매칭 검증)
            if (Slot->MountedItem.Item_Grade == EItemGrade::Rare)
            {
                for (const FString& Option : Slot->MountedItem.Item_Bonus_Options)
                {
                    if (Option.Contains(TEXT("STR+")))
                    {
                        FString NumberPart = Option.RightChop(Option.Find(TEXT("STR+")) + 4);
                        TotalBonusStr += FCString::Atoi(*NumberPart);
                    }
                    if (Option.Contains(TEXT("DEF+")))
                    {
                        FString NumberPart = Option.RightChop(Option.Find(TEXT("DEF+")) + 4);
                        TotalBonusDef += FCString::Atoi(*NumberPart);
                    }
                    if (Option.Contains(TEXT("MaxHP+")))
                    {
                        FString NumberPart = Option.RightChop(Option.Find(TEXT("MaxHP+")) + 6);
                        TotalBonusHp += FCString::Atoi(*NumberPart);
                    }
                }
            }
        }
    }
}

void UPTEquipmentComponent::OnRep_EquippedWeapon()
{
    // 리플리케이션을 통해 무기 데이터가 서버로부터 클라이언트에게 도착하면 실행됩니다.
    APTPlayerCharacter* OwnerCharacter = Cast<APTPlayerCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        if (EquippedWeapon.bIsEquipped)
        {
            // 장착 중이라면 복제되어 온 MountedItem의 메시를 손에 쥐여줍니다.
            OwnerCharacter->UpdateWeaponVisual(EquippedWeapon.MountedItem.ItemMeshAsset, EquippedWeapon.MountedItem);
        }
        else
        {
            // 해제 상태라면 손을 비웁니다.
            OwnerCharacter->UpdateWeaponVisual(TSoftObjectPtr<UStaticMesh>());
        }
    }
}
