#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Item/PTItemTypes.h"
#include "PTInventoryWidget.generated.h"

// class UUniformGridPanel;
// class UPTInventorySlotWidget;
// class UPTEquipSlotWidget;

UCLASS()
class PENTAGRAM_API UPTInventoryWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    //~ Begin UCommonActivatableWidget
    virtual void NativeOnInitialized() override;
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;
    //~ End UCommonActivatableWidget

public:
    /** 인벤토리 전체 데이터로 그리드 갱신. 아이템 시스템에서 배열 넘겨주면 됨. */
//     UFUNCTION(BlueprintCallable, Category = "PT|Inventory")
//     void RefreshInventory(const TArray<FInventorySlot>& InSlots);
//
//     /** 장비 슬롯 갱신 (Weapon/Chest). */
//     UFUNCTION(BlueprintCallable, Category = "PT|Inventory")
//     void RefreshEquipment(const FInventorySlot& Weapon, const FInventorySlot& Chest);
//
// protected:
//     /** 그리드 슬롯을 동적으로 생성. */
//     void BuildGrid();
//
// protected:
//     // ── BindWidget: UMG에서 같은 이름으로 배치 ──
//     UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
//     TObjectPtr<UUniformGridPanel> InventoryGrid;
//
//     UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
//     TObjectPtr<UPTEquipSlotWidget> EquipSlot_Weapon;
//
//     UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
//     TObjectPtr<UPTEquipSlotWidget> EquipSlot_Chest;
//
//     // ── 그리드 설정 ──
//     /** 그리드 슬롯에 사용할 위젯 클래스 (BP에서 지정). */
//     UPROPERTY(EditAnywhere, Category = "PT|Inventory")
//     TSubclassOf<UPTInventorySlotWidget> SlotWidgetClass;
//
//     UPROPERTY(EditAnywhere, Category = "PT|Inventory")
//     int32 Columns = 5;
//
//     UPROPERTY(EditAnywhere, Category = "PT|Inventory")
//     int32 Rows = 5;
//
//     /** 생성된 슬롯 위젯 캐시. */
//     UPROPERTY(Transient)
//     TArray<TObjectPtr<UPTInventorySlotWidget>> SlotWidgets;
};
