// PTCloseWidget.cpp
#include "PTCloseWidget.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Input/CommonUIInputTypes.h"

UPTCloseWidget::UPTCloseWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
}

void UPTCloseWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Quit)
    {
        Button_Quit->OnClicked.AddUniqueDynamic(this, &UPTCloseWidget::HandleQuitClicked);
    }

    if (Button_Close)
    {
        Button_Close->OnClicked.AddUniqueDynamic(this, &UPTCloseWidget::HandleCloseClicked);
    }

    if (DimBackgroundButton)
    {
        DimBackgroundButton->OnClicked.AddUniqueDynamic(this, &UPTCloseWidget::HandleCloseClicked);
    }
}

FReply UPTCloseWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return FReply::Handled();
}

TOptional<FUIInputConfig> UPTCloseWidget::GetDesiredInputConfig() const
{
    return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void UPTCloseWidget::HandleQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UPTCloseWidget::HandleCloseClicked()
{
    if (bCloseOnOutsideClick || Button_Close)
    {
        DeactivateWidget();
    }
}
