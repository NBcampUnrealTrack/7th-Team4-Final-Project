#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/PTItemTypes.h"
#include "PTEquipmentComponent.generated.h"

UENUM(BlueprintType)
enum class EEquipSlotType : uint8  // 장비 슬롯 타입
{
    Weapon    UMETA(DisplayName = "Weapon"),
    Chest     UMETA(DisplayName = "Chest")
};

USTRUCT(BlueprintType)
struct FEquipmentSlot // 장비 슬롯 구조체
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment") // 슬롯 타입
    EEquipSlotType EquippedSlotType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment") // 장착 여부
    bool bIsEquipped;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment") // 장착된 아이템 데이터
    FItemData MountedItem;

    FEquipmentSlot() : EquippedSlotType(EEquipSlotType::Weapon), bIsEquipped(false) {}
    FEquipmentSlot(EEquipSlotType InType) : EquippedSlotType(InType), bIsEquipped(false) {}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PENTAGRAM_API UPTEquipmentComponent : public UActorComponent // 장비창 컴포넌트 클래스
{
	GENERATED_BODY()

public:
	UPTEquipmentComponent();

protected:
	virtual void BeginPlay() override;

    // 네트워크 프로퍼티 복제를 위한 함수 오버라이드 
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override; 

public:
    // 장착 및 해제 함수 (서버 권한이 있으면 직접 연산, 클라이언트면 Server RPC 호출)
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool EquipItem(const FItemData& NewItem, FItemData& OutOldItem); // 장착 함수 (기존에 장착되어 있던 아이템 데이터를 OutOldItem, true 전부 반환)

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool UnequipItem(EEquipSlotType SlotType, FItemData& OutUnequippedItem); // 해제 함수 (해제된 아이템 데이터를 OutOldItem, true 전부 반환)

    // 클라이언트의 요청을 서버로 전달할 Server RPC 함수들 
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_EquipItem(const FItemData& NewItem);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UnequipItem(EEquipSlotType SlotType);

    // 실시간 보너스 스탯 게터(Getter) 함수들
    FORCEINLINE int32 GetTotalBonusStr() const { return TotalBonusStr; }
    FORCEINLINE int32 GetTotalBonusDef() const { return TotalBonusDef; }
    FORCEINLINE int32 GetTotalBonusHp() const { return TotalBonusHp; }

protected:
    // 슬롯 데이터
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment") // 장착된 무기 슬롯 저장
    FEquipmentSlot EquippedWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment") // 장착된 갑옷 슬롯 저장
    FEquipmentSlot EquippedChest;

    // 장착 중인 모든 장비의 스탯 합산
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment Stats")
    int32 TotalBonusStr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment Stats")
    int32 TotalBonusDef;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment Stats")
    int32 TotalBonusHp;

private:
    // 헬퍼함수 : 장착/해제 시 실시간으로 캐릭터 스탯 스냅샷을 갱신
    void UpdateTotalBonusStats();
};
