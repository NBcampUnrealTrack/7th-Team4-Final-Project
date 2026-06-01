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
    /** 슬롯에 데이터 주입. 아이템 시스템 완성되면 여기로 FInventorySlot 넘기면 됨. */
    UFUNCTION(BlueprintCallable, Category = "PT|Inventory")
    void SetSlotData(const FInventorySlot& InSlot);

    /** 슬롯 비우기. */
    UFUNCTION(BlueprintCallable, Category = "PT|Inventory")
    void ClearSlot();

    UFUNCTION(BlueprintPure, Category = "PT|Inventory")
    bool IsEmpty() const { return SlotData.IsEmpty(); }

    UFUNCTION(BlueprintPure, Category = "PT|Inventory")
    const FInventorySlot& GetSlotData() const { return SlotData; }

protected:
    /** 비주얼 갱신은 BP에서 구현 (아이콘/수량/등급테두리 등). */
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|Inventory")
    void OnRefreshVisual(const FInventorySlot& InSlot);

    /** 현재 슬롯이 들고 있는 데이터. */
    UPROPERTY(BlueprintReadOnly, Category = "PT|Inventory")
    FInventorySlot SlotData;

    /** 그리드 상의 인덱스 (몇 번째 칸인지). */
    UPROPERTY(BlueprintReadOnly, Category = "PT|Inventory")
    int32 SlotIndex = INDEX_NONE;

public:
    void SetSlotIndex(int32 InIndex) { SlotIndex = InIndex; }
    int32 GetSlotIndex() const { return SlotIndex; }
};
