#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/PTItemTypes.h"
#include "PTEquipmentComponent.generated.h"

UENUM(BlueprintType)
enum class EEquipSlotType : uint8 
{
    Weapon  UMETA(DisplayName = "Weapon"),
    Chest   UMETA(DisplayName = "Chest")
};

USTRUCT(BlueprintType)
struct FEquipmentSlot
{
    GENERATED_BODY()

    // 슬롯 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    EEquipSlotType EquippedSlotType;

    // 장착 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bIsEquipped;

    // 장착된 아이템 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FItemData MountedItem;

    FEquipmentSlot() : EquippedSlotType(EEquipSlotType::Weapon), bIsEquipped(false) {}
    FEquipmentSlot(EEquipSlotType InType) : EquippedSlotType(InType), bIsEquipped(false) {}
};

// 장비창 컴포넌트 클래스
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPTEquipmentComponent();

    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 장착 함수 (기존에 장착되어 있던 아이템 데이터를 OutOldItem, true 반환)
    // 서버 권한이 있으면 직접 연산, 클라이언트면 Server RPC 호출
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool EquipItem(const FItemData& NewItem, FItemData& OutOldItem);

    // 해제 함수 (해제된 아이템 데이터를 OutUnequippedItem, true 반환)
    // 서버 권한이 있으면 직접 연산, 클라이언트면 Server RPC 호출
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool UnequipItem(EEquipSlotType SlotType, FItemData& OutUnequippedItem);

    // ── RPC 함수 ─────────────────────────────────────────────────────────────

    // 클라이언트의 장착 요청을 서버로 전달
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_EquipItem(const FItemData& NewItem);

    // 클라이언트의 해제 요청을 서버로 전달
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UnequipItem(EEquipSlotType SlotType);

    // ── Getter 함수 ──────────────────────────────────────────────────────────

    FORCEINLINE int32 GetTotalBonusStr() const { return TotalBonusStr; }
    FORCEINLINE int32 GetTotalBonusDef() const { return TotalBonusDef; }
    FORCEINLINE int32 GetTotalBonusHp()  const { return TotalBonusHp;  }

protected:
    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void BeginPlay() override;

    // 네트워크 프로퍼티 복제를 위한 함수 오버라이드
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 헬퍼 함수 : 장착/해제 시 실시간으로 보너스 스탯 스냅샷을 갱신
    void UpdateTotalBonusStats();

protected:
    // ── 멤버 변수 (protected) ────────────────────────────────────────────────

    // 장착된 무기 슬롯
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment")
    FEquipmentSlot EquippedWeapon;

    // 장착된 갑옷 슬롯
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment")
    FEquipmentSlot EquippedChest;

    // 장착 중인 모든 장비의 스탯 합산
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment Stats")
    int32 TotalBonusStr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment Stats")
    int32 TotalBonusDef;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment Stats")
    int32 TotalBonusHp;
};
