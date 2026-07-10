#pragma once

#include "CoreMinimal.h"
#include "Audio/PTAudioSettings.h"
#include "CommonActivatableWidget.h"
#include "PTAudioSettingsWidget.generated.h"

class UButton;
class UProgressBar;
class USlider;
class UTextBlock;
class UPTAudioSubsystem;

UCLASS()
class PENTAGRAM_API UPTAudioSettingsWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Audio Settings")
    void ApplySettings();

    UFUNCTION(BlueprintCallable, Category = "PT|Audio Settings")
    void ResetSettingsToDefaults();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeOnActivated() override;

private:
    UFUNCTION()
    void HandleMasterVolumeChanged(float Value);

    UFUNCTION()
    void HandleBGMVolumeChanged(float Value);

    UFUNCTION()
    void HandleSFXVolumeChanged(float Value);

    UFUNCTION()
    void HandleUIVolumeChanged(float Value);

    UFUNCTION()
    void HandleAmbientVolumeChanged(float Value);

    UPTAudioSubsystem* ResolveAudioSubsystem() const;
    void RefreshFromAudioSubsystem();
    float QuantizeVolume(float Value) const;
    void SetChannelVolumeFromSlider(EPTAudioChannel Channel, USlider* Slider, float Value);
    void SetSliderValue(USlider* Slider, float Value) const;
    void SetProgressValue(UProgressBar* ProgressBar, float Value) const;
    void SetValueText(UTextBlock* TextBlock, float Value) const;
    void UpdateChannelDisplay(EPTAudioChannel Channel, float Value) const;

private:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> Slider_Master;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> Slider_BGM;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> Slider_SFX;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> Slider_UI;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> Slider_Ambient;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_MasterValue;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_BGMValue;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_SFXValue;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_UIValue;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_AmbientValue;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Progress_Master;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Progress_BGM;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Progress_SFX;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Progress_UI;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Progress_Ambient;

    bool bIsRefreshing = false;
};
