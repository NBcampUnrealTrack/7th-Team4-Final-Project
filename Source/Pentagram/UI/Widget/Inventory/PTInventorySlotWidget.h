// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/PTItemTypes.h"
#include "PTInventorySlotWidget.generated.h"

UCLASS()
class PENTAGRAM_API UPTInventorySlotWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // 데이터 주입
    UFUNCTION(BlueprintCallable, Category = "PT|Inventory")
    void SetSlotData(const FInventorySlot& InSlot);

    // 슬롯 비우기
    UFUNCTION(BlueprintCallable, Category = "PT|Inventory")
    void ClearSlot();

    UFUNCTION(BlueprintPure, Category = "PT|Inventory")
    bool IsEmpty() const { return SlotData.IsEmpty(); }

    UFUNCTION(BlueprintPure, Category = "PT|Inventory")
    const FInventorySlot& GetSlotData() const { return SlotData; }

    // 인덱스 접근자
    void SetSlotIndex(int32 InIndex) { SlotIndex = InIndex; }
    int32 GetSlotIndex() const { return SlotIndex; }

protected:
    // 비주얼 갱신
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|Inventory")
    void OnRefreshVisual(const FInventorySlot& InSlot);

    // 슬롯 데이터
    UPROPERTY(BlueprintReadOnly, Category = "PT|Inventory")
    FInventorySlot SlotData;

    // 그리드 인덱스
    UPROPERTY(BlueprintReadOnly, Category = "PT|Inventory")
    int32 SlotIndex = INDEX_NONE;
};
