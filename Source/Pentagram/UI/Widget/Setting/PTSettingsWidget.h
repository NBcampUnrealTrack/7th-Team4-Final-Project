#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTSettingsWidget.generated.h"

class UButton;
class UPTAudioSettingsWidget;
class UPTControlSettingsWidget;
class UPTDisplaySettingsWidget;
class UPTQuitConfirmWidget;
class UWidget;
class UWidgetSwitcher;

UENUM(BlueprintType)
enum class EPTSettingsPage : uint8
{
    Display UMETA(DisplayName = "Display"),
    Audio UMETA(DisplayName = "Audio"),
    Mouse UMETA(DisplayName = "Mouse")
};

UCLASS()
class PENTAGRAM_API UPTSettingsWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UPTSettingsWidget(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "PT|Settings")
    void SetActivePage(EPTSettingsPage Page);

    UFUNCTION(BlueprintPure, Category = "PT|Settings")
    EPTSettingsPage GetActivePage() const { return ActivePage; }

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "PT|Settings")
    void OnSettingsPageChanged(EPTSettingsPage NewPage);

private:
    UFUNCTION()
    void HandleDisplayClicked();

    UFUNCTION()
    void HandleAudioClicked();

    UFUNCTION()
    void HandleMouseClicked();

    UFUNCTION()
    void HandleCloseClicked();

    UFUNCTION()
    void HandleQuitGameClicked();

    UFUNCTION()
    void HandleResetDefaultsClicked();

    UFUNCTION()
    void HandleApplySettingsClicked();

    UFUNCTION()
    void HandleConfirmSettingsClicked();

    void HandleToggleSettingsAction();

    void BindButtons();
    void UnbindButtons();
    void ApplyActivePageSettings();
    void ResetActivePageSettingsToDefaults();
    UWidget* ResolvePageWidget(EPTSettingsPage Page) const;
    int32 ResolvePageIndex(EPTSettingsPage Page) const;
    void SetPageActiveState(EPTSettingsPage Page, bool bActive) const;

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Settings", meta = (AllowPrivateAccess = "true"))
    EPTSettingsPage DefaultPage = EPTSettingsPage::Display;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Settings", meta = (AllowPrivateAccess = "true"))
    EPTSettingsPage ActivePage = EPTSettingsPage::Display;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Settings", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UPTQuitConfirmWidget> QuitConfirmWidgetClass;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidgetSwitcher> Switcher_SettingPages;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidget> Page_Display;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidget> Page_Audio;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidget> Page_Mouse;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Display;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Audio;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Mouse;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Close;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Back;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_ResetDefaults;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_ApplySettings;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_ConfirmSettings;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_QuitGame;
};
