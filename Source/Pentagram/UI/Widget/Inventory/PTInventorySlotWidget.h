#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/PTItemTypes.h"
#include "PTInventorySlotWidget.generated.h"

class UImage;
class UUserWidget;
class UDragDropOperation;
class UPTItemTooltipWidget;

// 해제 요청
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnequipRequested, EItemType, EquipType, int32, ToIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnInventorySlotClicked, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnSlotUseRequested, int32, SlotIndex);

UCLASS()
class PENTAGRAM_API UPTInventorySlotWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // ── 델리게이트 ──
    UPROPERTY(BlueprintAssignable, Category = "PT|Inventory")
    FOnUnequipRequested OnUnequipRequested;

    UPROPERTY(BlueprintAssignable, Category = "PT|Inventory")
    FPTOnInventorySlotClicked OnClicked;

    UPROPERTY(BlueprintAssignable, Category = "PT|Inventory")
    FPTOnSlotUseRequested OnUseRequested;

    // ── 일반 함수 ──
    void SetSlotData(const FInventorySlot& InSlot);
    void ClearSlot();

    bool IsEmpty() const { return SlotData.IsEmpty(); }
    const FInventorySlot& GetSlotData() const { return SlotData; }

    void SetSlotIndex(int32 InIndex) { SlotIndex = InIndex; }
    int32 GetSlotIndex() const { return SlotIndex; }

protected:
    // ── 오버라이드 ──
    virtual void NativeOnInitialized() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    // ── 일반 함수 ──
    // 수락 검사
    virtual bool CanAccept(const FInventorySlot& InSlot) const { return true; }

    UFUNCTION()
    UWidget* GetItemToolTip();

    // ── 위젯 바인딩 ──
    UPROPERTY(meta = (BindWidget))
    UImage* Img_Icon;

    // ── 설정 ──
    UPROPERTY(EditAnywhere, Category = "Inventory")
    TSubclassOf<UPTItemTooltipWidget> ToolTipClass;

    // 드래그 비주얼
    UPROPERTY(EditAnywhere, Category = "Inventory")
    TSubclassOf<UUserWidget> DragVisualClass;

private:
    // ── 일반 함수 ──
    void RefreshIcon();
    UWidget* CreateDragVisual();

    // ── 멤버 변수 ──
    FInventorySlot SlotData;

    int32 SlotIndex = INDEX_NONE;
};
