// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTShopWidget.generated.h"

class APTBasePlayerState;
class APTShopNPCCharacter;
class UButton;
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

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

    UFUNCTION()
    void HandleCloseClicked();

    UFUNCTION()
    void HandleBuyRequested(int32 SlotIndex);

    UFUNCTION()
    void HandleGoldChanged(int64 NewAmount);

    void BuildProductList();
    void RefreshGold();
    void BindPlayerStateDelegates();
    void UnbindPlayerStateDelegates();

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop")
    TObjectPtr<APTShopNPCCharacter> TargetShopNPC;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UUniformGridPanel> ProductGrid;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_CurrentGold;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Close;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Shop")
    TSubclassOf<UPTShopSlotWidget> ShopSlotWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Shop", meta = (ClampMin = "1"))
    int32 ColumnCount = 5;

private:
    TArray<FName> DisplayedProductIDs;
};
