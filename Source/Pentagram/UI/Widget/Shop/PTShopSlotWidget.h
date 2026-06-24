// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/PTItemTypes.h"
#include "UI/Data/PTDelegates.h"
#include "PTShopSlotWidget.generated.h"

class UTextBlock;
class UImage;

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTShopSlotWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // 상점 주입
    UFUNCTION(BlueprintCallable, Category = "PT|Shop")
    void SetShopData(const FInventorySlot& InSlot, int32 InPrice);

    // 가격 주입
    UFUNCTION(BlueprintCallable, Category = "PT|Shop")
    void SetPrice(int32 InPrice);

    // 슬롯 비우기
    UFUNCTION(BlueprintCallable, Category = "PT|Shop")
    void ClearSlot();

    // 기존 Blueprint 연결을 유지하기 위한 상품 선택 요청
    UFUNCTION(BlueprintCallable, Category = "PT|Shop")
    void RequestBuy();

    UFUNCTION(BlueprintPure, Category = "PT|Shop")
    bool IsEmpty() const { return SlotData.IsEmpty(); }

    UFUNCTION(BlueprintPure, Category = "PT|Shop")
    const FInventorySlot& GetSlotData() const { return SlotData; }

    UFUNCTION(BlueprintPure, Category = "PT|Shop")
    int32 GetPrice() const { return Price; }

    // 인덱스 접근자
    void SetSlotIndex(int32 InIndex) { SlotIndex = InIndex; }
    int32 GetSlotIndex() const { return SlotIndex; }

    // 상품 선택 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "PT|Shop")
    FPTOnShopBuyRequested OnSelected;

    UPROPERTY(BlueprintAssignable, Category = "PT|Shop")
    FPTOnShopSlotHoverEvent OnHovered;

    UPROPERTY(BlueprintAssignable, Category = "PT|Shop")
    FPTOnShopSlotHoverEvent OnUnhovered;

protected:
    virtual void NativeOnMouseEnter(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    virtual void NativeOnMouseLeave(
        const FPointerEvent& InMouseEvent) override;

    // 비주얼 갱신
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|Shop")
    void OnRefreshVisual(const FInventorySlot& InSlot);

    // 가격 갱신
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|Shop")
    void OnRefreshPrice(int32 InPrice);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_ItemName;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop", meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_Icon;

    // 슬롯 데이터
    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop")
    FInventorySlot SlotData;

    // 그리드 인덱스
    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop")
    int32 SlotIndex = INDEX_NONE;

    // 가격
    UPROPERTY(BlueprintReadOnly, Category = "PT|Shop")
    int32 Price = 0;
};
