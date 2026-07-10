#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTQuitConfirmWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class PENTAGRAM_API UPTQuitConfirmWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeOnActivated() override;

private:
    UFUNCTION()
    void HandleConfirmClicked();

    UFUNCTION()
    void HandleCancelClicked();

    UFUNCTION()
    void HandleConfirmHovered();

    UFUNCTION()
    void HandleCancelHovered();

    UFUNCTION()
    void HandleButtonUnhovered();

    void SetDialogTexture(UTexture2D* Texture);

private:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_Dialog;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_ConfirmQuit;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_CancelQuit;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Message;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quit", meta = (AllowPrivateAccess = "true"))
    FText QuitMessage = NSLOCTEXT("PTQuitConfirmWidget", "QuitMessage", "게임을 종료하시겠습니까?");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Quit|Images", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UTexture2D> DialogNormalTexture;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Quit|Images", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UTexture2D> DialogCancelHoverTexture;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Quit|Images", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UTexture2D> DialogConfirmHoverTexture;
};
