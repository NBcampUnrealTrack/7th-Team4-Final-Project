#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/PTItemTypes.h"
#include "Item/PTDropItemActorBase.h"
#include "PTInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnQuickSlotChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPTInventoryComponent();

    // 아이템 추가 시도 함수 (성공 시 true, 가방이 가득 차면 false)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItem(const FItemData& NewItemData, int32 Count = 1);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool CanAddItem(const FItemData& NewItemData, int32 Count = 1) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemCount(FName ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemID, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemAtSlot(int32 SlotIndex, int32 Count = 1);

    // 인벤토리 슬롯(UI)
    UFUNCTION(BlueprintPure, Category = "Inventory")
    const TArray<FInventorySlot>& GetInventorySlots() const { return InventorySlots; }

    bool RestoreInventorySlots(const TArray<FInventorySlot>& InInventorySlots);

    // 블루프린트나 캐릭터에서 호출할 물약 사용 함수
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UsePotion(int32 SlotIndex);

    // 디버깅용 : 현재 인벤토리 상태를 로그창에 출력
    void PrintInventoryLog();

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FPTOnInventoryChanged OnInventoryChanged;


    // [리플리케이션] 클라이언트가 물약을 먹었을 때 서버에게 실제 데이터 처리를 요청하는 Server RPC
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UsePotion(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItemAtSlot(int32 SlotIndex);

    bool UseSkillBook(int32 SlotIndex);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UseSkillBook(int32 SlotIndex);

public:

    virtual void BeginPlay() override;

    // 네트워크 리플리케이트를 위한 프로퍼티 등록 함수 오버라이드
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_InventorySlots();

    // 가방 크기 총 30칸
    const int32 MaxSlotCount = 30;

    // 인벤토리 실제 데이터를 담는 배열
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_InventorySlots, Category = "Inventory")
    TArray<FInventorySlot> InventorySlots;

    // ===== QuickSlot (소모품 전용) =====
    static constexpr int32 QuickSlotCount = 2;

    // 각 퀵슬롯에 등록된 소모품의 Item_ID (인덱스 0 = 1번키, 1 = 2번키)
    UPROPERTY(ReplicatedUsing = OnRep_QuickSlots, VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
    TArray<FName> QuickSlots;

    UPROPERTY(BlueprintAssignable, Category = "QuickSlot")
    FPTOnQuickSlotChanged OnQuickSlotChanged;

    // UI가 호출: 인벤토리 슬롯의 아이템을 퀵슬롯에 등록 (소모품만 허용)
    UFUNCTION(BlueprintCallable, Category = "QuickSlot")
    bool RegisterConsumableToQuickSlot(int32 QuickIndex, int32 InventorySlotIndex);

    UFUNCTION(BlueprintCallable, Category = "QuickSlot")
    void ClearQuickSlot(int32 QuickIndex);

    // 키(1/2) 입력 시 호출: 등록된 소모품 사용
    UFUNCTION(BlueprintCallable, Category = "QuickSlot")
    bool UseQuickSlot(int32 QuickIndex);

    UFUNCTION(BlueprintPure, Category = "QuickSlot")
    FName GetQuickSlotItemID(int32 QuickIndex) const;

    UFUNCTION()
    void OnRep_QuickSlots();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RegisterConsumableToQuickSlot(int32 QuickIndex, FName ItemID);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ClearQuickSlot(int32 QuickIndex);

private:

    // 소비 아이템용: 동일한 아이템 ID를 가진 슬롯의 인덱스를 반환 (없으면 INDEX_NONE)
    int32 FindSameItemSlot(const FName& ItemID) const;

    // 빈 슬롯의 인덱스를 반환 (없으면 INDEX_NONE)
    int32 FindEmptySlot() const;

    // 실제 캐릭터를 찾아서 피를 채워줄 내부 틱 함수 (5초간 매초 실행)
    void ExecutePotionHealing();
    void NotifyQuestItemCollected(const FItemData& ItemData, int32 Count) const;
    void BroadcastInventoryChanged();
    void SaveOwnerPlayerState() const;


    // 포션 회복 타이머 핸들
    FTimerHandle PotionTimerHandle;

    // 포션 회복 틱 카운터
    int32 PotionTickCount = 0;
};
