// Fill out your copyright notice in the Description page of Project Settings.

#include "PTShopSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPTShopSlotWidget::NativeOnMouseEnter(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (!IsEmpty())
    {
        OnHovered.Broadcast(SlotIndex);
    }
}

void UPTShopSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    if (!IsEmpty())
    {
        OnUnhovered.Broadcast(SlotIndex);
    }
}

void UPTShopSlotWidget::SetShopData(const FInventorySlot& InSlot, int32 InPrice)
{
    SlotData = InSlot;

    if (Img_Icon != nullptr)
    {
        if (!SlotData.IsEmpty() && !SlotData.ItemData.Item_Icon.IsNull())
        {
            Img_Icon->SetBrushFromSoftTexture(SlotData.ItemData.Item_Icon, true);
        }
        else
        {
            Img_Icon->SetBrushFromTexture(nullptr);
        }
    }

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

    if (Img_Icon != nullptr)
    {
        Img_Icon->SetBrushFromTexture(nullptr);
    }

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

    OnSelected.Broadcast(SlotIndex);
}
