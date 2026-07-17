// PTGuideWidget.cpp
#include "UI/Setting/PTGuideWidget.h"
#include "UI/Data/PTDelegates.h"

void UPTGuideWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 전체클릭받게
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);
}

void UPTGuideWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    SetFocus();
}

FReply UPTGuideWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bDismissOnAnyClick)
    {
        DismissGuide();
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPTGuideWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InTouchEvent)
{
    if (bDismissOnAnyClick)
    {
        DismissGuide();
        return FReply::Handled();
    }

    return Super::NativeOnTouchStarted(InGeometry, InTouchEvent);
}

UWidget* UPTGuideWidget::NativeGetDesiredFocusTarget() const
{
    // 이 위젯자체가 포커스타겟 (전체클릭수신용)
    return const_cast<UPTGuideWidget*>(this);
}
FReply UPTGuideWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (bDismissOnAnyClick)
    {
        DismissGuide();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
void UPTGuideWidget::DismissGuide()
{
    // 스택에서제거
    DeactivateWidget();
}
