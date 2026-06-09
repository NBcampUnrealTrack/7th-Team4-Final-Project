// Fill out your copyright notice in the Description page of Project Settings.

#include "PTInventorySlotWidget.h"

void UPTInventorySlotWidget::SetSlotData(const FInventorySlot& InSlot)
{
    SlotData = InSlot;
    OnRefreshVisual(SlotData); // BP 그리기
}

void UPTInventorySlotWidget::ClearSlot()
{
    SlotData = FInventorySlot();
    OnRefreshVisual(SlotData);
}
