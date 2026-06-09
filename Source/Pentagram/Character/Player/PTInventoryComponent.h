#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/PTItemTypes.h"
#include "Item/PTDropItemActorBase.h"
#include "PTInventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPTInventoryComponent();

    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 아이템 추가 시도 함수 (성공 시 true, 가방이 가득 차면 false)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItem(const FItemData& NewItemData, int32 Count = 1);

    // 블루프린트나 캐릭터에서 호출할 물약 사용 함수
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UsePotion(int32 SlotIndex);

    // 디버깅용 : 현재 인벤토리 상태를 로그창에 출력
    void PrintInventoryLog();

    // ── RPC 함수 ─────────────────────────────────────────────────────────────

    // [리플리케이션] 클라이언트가 물약을 먹었을 때 서버에게 실제 데이터 처리를 요청하는 Server RPC
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UsePotion(int32 SlotIndex);

protected:
    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void BeginPlay() override;

    // 네트워크 리플리케이트를 위한 프로퍼티 등록 함수 오버라이드
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    // ── 멤버 변수 (protected) ────────────────────────────────────────────────

    // 가방 크기 총 30칸
    const int32 MaxSlotCount = 30;

    // 인벤토리 실제 데이터를 담는 배열
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Inventory")
    TArray<FInventorySlot> InventorySlots;

private:
    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 소비 아이템용: 동일한 아이템 ID를 가진 슬롯의 인덱스를 반환 (없으면 INDEX_NONE)
    int32 FindSameItemSlot(const FName& ItemID) const;

    // 빈 슬롯의 인덱스를 반환 (없으면 INDEX_NONE)
    int32 FindEmptySlot() const;

    // 실제 캐릭터를 찾아서 피를 채워줄 내부 틱 함수 (5초간 매초 실행)
    void ExecutePotionHealing();

    // ── 멤버 변수 (private) ──────────────────────────────────────────────────

    // 포션 회복 타이머 핸들
    FTimerHandle PotionTimerHandle;

    // 포션 회복 틱 카운터
    int32 PotionTickCount = 0;
};
