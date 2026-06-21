// Fill out your copyright notice in the Description page of Project Settings.

#include "PTItemTooltipWidget.h"
#include "Components/TextBlock.h"

void UPTItemTooltipWidget::SetItem(const FInventorySlot& InSlot)
{
    if (InSlot.IsEmpty())
    {
        Clear();
        return;
    }

    const FItemData& Data = InSlot.ItemData;

    if (Txt_Name)     Txt_Name->SetText(Data.Item_Name);
    if (Txt_Grade)    Txt_Grade->SetText(FText::FromString(EnumToString(StaticEnum<EItemGrade>(), (int64)Data.Item_Grade)));
    if (Txt_Type)     Txt_Type->SetText(FText::FromString(EnumToString(StaticEnum<EItemType>(), (int64)Data.Item_Type)));
    if (Txt_BaseStat) Txt_BaseStat->SetText(FText::AsNumber(Data.Item_Base_Stat));
    if (Txt_Options)  Txt_Options->SetText(FText::FromString(FString::Join(Data.Item_Bonus_Options, LINE_TERMINATOR)));
}

void UPTItemTooltipWidget::Clear()
{
    if (Txt_Name)     Txt_Name->SetText(FText::GetEmpty());
    if (Txt_Grade)    Txt_Grade->SetText(FText::GetEmpty());
    if (Txt_Type)     Txt_Type->SetText(FText::GetEmpty());
    if (Txt_BaseStat) Txt_BaseStat->SetText(FText::GetEmpty());
    if (Txt_Options)  Txt_Options->SetText(FText::GetEmpty());
}

FString UPTItemTooltipWidget::EnumToString(const UEnum* EnumPtr, int64 Value)
{
    return EnumPtr ? EnumPtr->GetDisplayNameTextByValue(Value).ToString() : FString();
}
