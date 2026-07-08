#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Item/PTItemTypes.h"
#include "Interface/PTUIContentBoundsInterface.h" // 실제 경로에 맞게 수정
#include "PTInventoryWidget.generated.h"

class UUniformGridPanel;
class UPTInventorySlotWidget;
class UPTInventoryComponent;
class UPTEquipPanelWidget;
class UPTShopWidget;

UCLASS()
class PENTAGRAM_API UPTInventoryWidget : public UCommonActivatableWidget, public IPTUIContentBoundsInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Inventory|Shop")
    void SetShopSellTarget(UPTShopWidget* InShopWidget);

    UFUNCTION(BlueprintCallable, Category = "PT|Inventory|Shop")
    void ClearShopSellTarget();

     virtual bool IsScreenPositionOverContent_Implementation(const FVector2D& ScreenPosition) const override;

protected:
    // ── 오버라이드 ──
    virtual void NativeOnInitialized() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;


    // ── 일반 함수 ──
    void BuildSlots();

    UFUNCTION()
    void RefreshAllSlots();
    void BindInventoryChanged();
    void UnbindInventoryChanged();

    UFUNCTION()
    void HandleInventoryChanged();

    UFUNCTION()
    void HandleSlotClicked(int32 SlotIndex);

    UFUNCTION()
    void HandleEquipRequested(int32 FromIndex, EItemType EquipType);

    UFUNCTION()
    void HandleUnequipRequested(EItemType EquipType, int32 ToIndex);

    // ── 위젯 바인딩 ──
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* InventoryGrid;

    UPROPERTY(meta = (BindWidget))
    UPTEquipPanelWidget* EquipPanel;

    // ── 설정 ──
    UPROPERTY(EditAnywhere, Category = "Inventory")
    TSubclassOf<UPTInventorySlotWidget> SlotClass;

    UPROPERTY(EditAnywhere, Category = "Inventory")
    int32 SlotCount = 30;   // 컴포넌트(30칸)와 맞춤

    UPROPERTY(EditAnywhere, Category = "Inventory")
    int32 Columns = 6;

    // ── 멤버 변수 ──
    UPROPERTY(Transient)
    TArray<TObjectPtr<UPTInventorySlotWidget>> SlotWidgets;

    UPROPERTY(Transient)
    TObjectPtr<UPTShopWidget> ShopWidgetForSell;

    TWeakObjectPtr<UPTInventoryComponent> BoundInventoryComponent;


private:
    UPTInventoryComponent* ResolveInventoryComponent() const;
};
