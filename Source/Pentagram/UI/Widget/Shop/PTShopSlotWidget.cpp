// Fill out your copyright notice in the Description page of Project Settings.

#include "PTShopSlotWidget.h"

#include "Components/TextBlock.h"

void UPTShopSlotWidget::SetShopData(const FInventorySlot& InSlot, int32 InPrice)
{
    SlotData = InSlot;

    if (Txt_ItemName != nullptr)
    {
        Txt_ItemName->SetText(SlotData.IsEmpty() ? FText::GetEmpty() : SlotData.ItemData.Item_Name);
    }

    OnRefreshVisual(SlotData); // 아이템 갱신
    SetPrice(InPrice);
}

void UPTShopSlotWidget::SetPrice(int32 InPrice)
{
    Price = InPrice;
    OnRefreshPrice(Price); // 가격 갱신
}

void UPTShopSlotWidget::ClearSlot()
{
    SlotData = FInventorySlot();
    Price = 0;

    if (Txt_ItemName != nullptr)
    {
        Txt_ItemName->SetText(FText::GetEmpty());
    }

    OnRefreshVisual(SlotData);
    OnRefreshPrice(Price);
}

void UPTShopSlotWidget::RequestBuy()
{
    if (IsEmpty())
    {
        return;
    }

    OnBuyClicked.Broadcast(SlotIndex); // 구매 통지
}
