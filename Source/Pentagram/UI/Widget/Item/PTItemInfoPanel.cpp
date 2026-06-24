#include "PTItemInfoPanel.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPTItemInfoPanel::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    ClearItemData();
}

void UPTItemInfoPanel::SetItemData(const FItemData& InItemData)
{
    ItemData = InItemData;

    if (Img_ItemPreview != nullptr)
    {
        Img_ItemPreview->SetBrushFromSoftTexture(ItemData.Item_Icon, true);
    }

    if (Txt_ItemName != nullptr)
    {
        Txt_ItemName->SetText(ItemData.Item_Name);
    }

    if (Txt_ItemType != nullptr)
    {
        Txt_ItemType->SetText(MakeItemTypeText());
    }

    if (Txt_ItemGrade != nullptr)
    {
        Txt_ItemGrade->SetText(MakeItemGradeText());
    }

    if (Txt_BaseStat != nullptr)
    {
        Txt_BaseStat->SetText(MakeBaseStatText());
    }

    if (Txt_Options != nullptr)
    {
        Txt_Options->SetText(MakeOptionsText());
    }

    if (Txt_Description != nullptr)
    {
        Txt_Description->SetText(ItemData.Item_Description);
    }

    SetVisibility(ESlateVisibility::Visible);
    OnRefreshItemInfo(ItemData);
}

void UPTItemInfoPanel::ClearItemData()
{
    ItemData = FItemData();

    if (Img_ItemPreview != nullptr)
    {
        Img_ItemPreview->SetBrushFromTexture(nullptr);
    }

    if (Txt_ItemName != nullptr)
    {
        Txt_ItemName->SetText(FText::GetEmpty());
    }

    if (Txt_ItemType != nullptr)
    {
        Txt_ItemType->SetText(FText::GetEmpty());
    }

    if (Txt_ItemGrade != nullptr)
    {
        Txt_ItemGrade->SetText(FText::GetEmpty());
    }

    if (Txt_BaseStat != nullptr)
    {
        Txt_BaseStat->SetText(FText::GetEmpty());
    }

    if (Txt_Options != nullptr)
    {
        Txt_Options->SetText(FText::GetEmpty());
    }

    if (Txt_Description != nullptr)
    {
        Txt_Description->SetText(FText::GetEmpty());
    }

    SetVisibility(ESlateVisibility::Collapsed);
}

FText UPTItemInfoPanel::MakeItemTypeText() const
{
    const UEnum* ItemTypeEnum = StaticEnum<EItemType>();
    return ItemTypeEnum != nullptr
        ? ItemTypeEnum->GetDisplayNameTextByValue(static_cast<int64>(ItemData.Item_Type))
        : FText::GetEmpty();
}

FText UPTItemInfoPanel::MakeItemGradeText() const
{
    if (ItemData.Item_Grade == EItemGrade::None)
    {
        return FText::GetEmpty();
    }

    const UEnum* ItemGradeEnum = StaticEnum<EItemGrade>();
    return ItemGradeEnum != nullptr
        ? ItemGradeEnum->GetDisplayNameTextByValue(static_cast<int64>(ItemData.Item_Grade))
        : FText::GetEmpty();
}

FText UPTItemInfoPanel::MakeBaseStatText() const
{
    if (ItemData.Item_Base_Stat == 0)
    {
        return FText::GetEmpty();
    }

    const FText StatLabel = ItemData.Item_Type == EItemType::Weapon
        ? NSLOCTEXT("PTItemInfo", "DamageLabel", "Damage")
        : NSLOCTEXT("PTItemInfo", "BaseStatLabel", "Base Stat");

    return FText::Format(
        NSLOCTEXT("PTItemInfo", "BaseStatFormat", "{0} {1}"),
        FText::AsNumber(ItemData.Item_Base_Stat),
        StatLabel);
}

FText UPTItemInfoPanel::MakeOptionsText() const
{
    FString OptionsString;

    for (const FString& Option : ItemData.Item_Bonus_Options)
    {
        if (Option.IsEmpty())
        {
            continue;
        }

        if (!OptionsString.IsEmpty())
        {
            OptionsString.AppendChar(TEXT('\n'));
        }

        OptionsString.Append(Option);
    }

    return FText::FromString(OptionsString);
}
