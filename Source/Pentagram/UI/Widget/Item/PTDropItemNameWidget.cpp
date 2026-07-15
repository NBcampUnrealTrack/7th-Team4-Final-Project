#include "PTDropItemNameWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void UPTDropItemNameWidget::SetItemName(const FText& InItemName)
{
    DisplayItemName = InItemName;
    RefreshItemName();
}

TSharedRef<SWidget> UPTDropItemNameWidget::RebuildWidget()
{
    if (WidgetTree != nullptr && WidgetTree->RootWidget == nullptr)
    {
        Txt_ItemName = WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(),
            TEXT("Txt_ItemName"));
        WidgetTree->RootWidget = Txt_ItemName;

        FSlateFontInfo Font = Txt_ItemName->GetFont();
        Font.Size = 18;
        Txt_ItemName->SetFont(Font);
        Txt_ItemName->SetJustification(ETextJustify::Center);
        Txt_ItemName->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        Txt_ItemName->SetShadowOffset(FVector2D(1.5f, 1.5f));
        Txt_ItemName->SetShadowColorAndOpacity(FLinearColor::Black);
        Txt_ItemName->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    return Super::RebuildWidget();
}

void UPTDropItemNameWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
    SetVisibility(ESlateVisibility::HitTestInvisible);
    RefreshItemName();
}

void UPTDropItemNameWidget::RefreshItemName()
{
    if (Txt_ItemName != nullptr)
    {
        Txt_ItemName->SetText(DisplayItemName);
    }
}
