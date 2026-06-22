#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/PTItemTypes.h"
#include "PTEquipPanelWidget.generated.h"

class UPTEquipSlotWidget;

// 장착 요청
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipPanelRequested, int32, FromIndex, EItemType, EquipType);

UCLASS()
class PENTAGRAM_API UPTEquipPanelWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // ── 델리게이트 ──
    UPROPERTY(BlueprintAssignable, Category = "PT|Equip")
    FOnEquipPanelRequested OnEquipRequested;

    // ── 일반 함수 ──
    void RefreshSlot(EItemType Type, const FInventorySlot& InSlot);
    void ClearSlot(EItemType Type);

protected:
    // ── 오버라이드 ──
    virtual void NativeOnInitialized() override;

    // ── 일반 함수 ──
    UFUNCTION()
    void HandleEquipRequested(int32 FromIndex, EItemType EquipType);

private:
    // ── 일반 함수 ──
    void CollectSlots();

    // ── 멤버 변수 ──
    UPROPERTY()
    TMap<EItemType, UPTEquipSlotWidget*> SlotMap;
};
