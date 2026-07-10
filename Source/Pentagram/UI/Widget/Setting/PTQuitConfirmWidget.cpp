#include "UI/Widget/Setting/PTQuitConfirmWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Kismet/KismetSystemLibrary.h"

void UPTQuitConfirmWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_ConfirmQuit != nullptr)
    {
        Btn_ConfirmQuit->OnClicked.AddDynamic(this, &UPTQuitConfirmWidget::HandleConfirmClicked);
        Btn_ConfirmQuit->OnHovered.AddDynamic(this, &UPTQuitConfirmWidget::HandleConfirmHovered);
        Btn_ConfirmQuit->OnUnhovered.AddDynamic(this, &UPTQuitConfirmWidget::HandleButtonUnhovered);
    }

    if (Btn_CancelQuit != nullptr)
    {
        Btn_CancelQuit->OnClicked.AddDynamic(this, &UPTQuitConfirmWidget::HandleCancelClicked);
        Btn_CancelQuit->OnHovered.AddDynamic(this, &UPTQuitConfirmWidget::HandleCancelHovered);
        Btn_CancelQuit->OnUnhovered.AddDynamic(this, &UPTQuitConfirmWidget::HandleButtonUnhovered);
    }

    if (Text_Message != nullptr)
    {
        Text_Message->SetText(QuitMessage);
    }

    SetDialogTexture(DialogNormalTexture);
}

void UPTQuitConfirmWidget::NativeDestruct()
{
    if (Btn_ConfirmQuit != nullptr)
    {
        Btn_ConfirmQuit->OnClicked.RemoveDynamic(this, &UPTQuitConfirmWidget::HandleConfirmClicked);
        Btn_ConfirmQuit->OnHovered.RemoveDynamic(this, &UPTQuitConfirmWidget::HandleConfirmHovered);
        Btn_ConfirmQuit->OnUnhovered.RemoveDynamic(this, &UPTQuitConfirmWidget::HandleButtonUnhovered);
    }

    if (Btn_CancelQuit != nullptr)
    {
        Btn_CancelQuit->OnClicked.RemoveDynamic(this, &UPTQuitConfirmWidget::HandleCancelClicked);
        Btn_CancelQuit->OnHovered.RemoveDynamic(this, &UPTQuitConfirmWidget::HandleCancelHovered);
        Btn_CancelQuit->OnUnhovered.RemoveDynamic(this, &UPTQuitConfirmWidget::HandleButtonUnhovered);
    }

    Super::NativeDestruct();
}

void UPTQuitConfirmWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    if (Text_Message != nullptr)
    {
        Text_Message->SetText(QuitMessage);
    }

    SetDialogTexture(DialogNormalTexture);
}

void UPTQuitConfirmWidget::HandleConfirmClicked()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UPTQuitConfirmWidget::HandleCancelClicked()
{
    DeactivateWidget();
}

void UPTQuitConfirmWidget::HandleConfirmHovered()
{
    SetDialogTexture(DialogConfirmHoverTexture);
}

void UPTQuitConfirmWidget::HandleCancelHovered()
{
    SetDialogTexture(DialogCancelHoverTexture);
}

void UPTQuitConfirmWidget::HandleButtonUnhovered()
{
    SetDialogTexture(DialogNormalTexture);
}

void UPTQuitConfirmWidget::SetDialogTexture(UTexture2D* Texture)
{
    if (Image_Dialog == nullptr || Texture == nullptr)
    {
        return;
    }

    Image_Dialog->SetBrushFromTexture(Texture, true);
}
