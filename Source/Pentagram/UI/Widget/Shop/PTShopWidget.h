// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Item/PTItemTypes.h"
#include "PTShopWidget.generated.h"

class APTBasePlayerState;
class APTShopNPCCharacter;
class UButton;
class UPTItemInfoPanel;
class UPTShopSlotWidget;
class UTextBlock;
class UUniformGridPanel;

UCLASS()
class PENTAGRAM_API UPTShopWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Shop")
    void SetupShop(APTShopNPCCharacter* InShopNPC);

    UFUNCTION(BlueprintCallable, Category = "PT|Shop")
    void SelectInventoryItemForSell(int32 InventorySlotIndex, const FInventorySlot& SlotData);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

    UFUNCTION()
    void HandleCloseClicked();

    UFUNCTION()
    void HandleProductSelected(int32 SlotIndex);

    UFUNCTION()
    void HandleProductHovered(int32 SlotIndex);

    UFUNCTION()
    void HandleProductUnhovered(int32 SlotIndex);

    UFUNCTION()
    void HandleBuyClicked();

    UFUNCTION()
    void HandleGoldChanged(int64 NewAmount);

    void BuildProductList();
    void ResetProductSelection();
    void SetActionButtonText(const FText& InText);
    void PositionItemInfoPanel(int32 SlotIndex);
    void RefreshGold();
    void BindPlayerStateDelegates();
    void UnbindPlayerStateDelegates();

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop")
    TObjectPtr<APTShopNPCCharacter> TargetShopNPC;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop", meta = (BindWidgetOptional))
    TObjectPtr<UUniformGridPanel> ProductGrid;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_CurrentGold;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_SelectedPrice;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop", meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Close;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop", meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Buy;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_BuyLabel;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop", meta = (BindWidgetOptional))
    TObjectPtr<UPTItemInfoPanel> ItemInfoPanel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Shop")
    TSubclassOf<UPTShopSlotWidget> ShopSlotWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Shop", meta = (ClampMin = "1"))
    int32 ColumnCount = 5;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Shop|Item Info",
        meta = (ClampMin = "0.0", UIMin = "0.0"))
    float ItemInfoPanelGap = 16.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Shop|Item Info")
    bool bPreferItemInfoPanelOnLeft = true;

private:
    TArray<FName> DisplayedProductIDs;
    TArray<int32> DisplayedProductPrices;
    TArray<FItemData> DisplayedProductData;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UPTShopSlotWidget>> DisplayedProductSlots;

    int32 SelectedSlotIndex = INDEX_NONE;
    int32 SelectedSellInventorySlotIndex = INDEX_NONE;
    FName SelectedSellItemID = NAME_None;
    int32 HoveredSlotIndex = INDEX_NONE;
};
