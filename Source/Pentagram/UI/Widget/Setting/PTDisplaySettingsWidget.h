#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/GameUserSettings.h"
#include "PTDisplaySettingsWidget.generated.h"

class UButton;
class UCheckBox;
class UProgressBar;
class USlider;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTDisplaySettingsWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Display Settings")
    void ApplySettings();

    UFUNCTION(BlueprintCallable, Category = "PT|Display Settings")
    void ResetSettingsToDefaults();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeOnActivated() override;

private:
    UFUNCTION()
    void HandleWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleVSyncChanged(bool bIsChecked);

    UFUNCTION()
    void HandleFrameLimitChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleBrightnessChanged(float Value);

    void PopulateOptionLists();
    void PopulateWindowModeOptions();
    void PopulateResolutionOptions();
    void PopulateFrameLimitOptions();
    void PopulateQualityOptions();

    void RefreshFromGameUserSettings();
    void ApplyDisplaySettings();
    void ApplyBrightness(float Brightness) const;
    float LoadBrightness() const;
    void SaveBrightness(float Brightness) const;

    FString MakeResolutionLabel(const FIntPoint& Resolution) const;
    FString MakeFrameLimitLabel(float FrameLimit) const;
    FString MakeQualityLabel(int32 QualityLevel) const;
    FString MakeWindowModeLabel(EWindowMode::Type WindowMode) const;

    float QuantizeNormalizedValue(float Value) const;
    void SetSelectedOption(UComboBoxString* ComboBox, const FString& Option) const;
    void SetBrightnessDisplay(float Brightness) const;

private:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UComboBoxString> Combo_WindowMode;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UComboBoxString> Combo_Resolution;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBox_VSync;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UComboBoxString> Combo_FrameLimit;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UComboBoxString> Combo_Quality;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> Slider_Brightness;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Progress_Brightness;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_BrightnessValue;

    TMap<FString, EWindowMode::Type> WindowModeOptions;
    TMap<FString, FIntPoint> ResolutionOptions;
    TMap<FString, float> FrameLimitOptions;
    TMap<FString, int32> QualityOptions;

    EWindowMode::Type PendingWindowMode = EWindowMode::WindowedFullscreen;
    FIntPoint PendingResolution = FIntPoint(1920, 1080);
    bool bPendingVSync = false;
    float PendingFrameLimit = 0.f;
    int32 PendingQualityLevel = 3;
    float PendingBrightness = 0.5f;
    bool bIsRefreshing = false;
};
