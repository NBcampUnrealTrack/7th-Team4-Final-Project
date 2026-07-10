#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTControlSettingsWidget.generated.h"

class UButton;
class UProgressBar;
class USlider;
class UTextBlock;
class UPTControlSettingsSubsystem;

UCLASS()
class PENTAGRAM_API UPTControlSettingsWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Control Settings")
    void ApplySettings();

    UFUNCTION(BlueprintCallable, Category = "PT|Control Settings")
    void ResetSettingsToDefaults();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleMouseSensitivityChanged(float Value);

    UPTControlSettingsSubsystem* ResolveControlSettingsSubsystem() const;
    void RefreshFromControlSettings();
    float QuantizeNormalizedValue(float Value) const;
    void SetMouseSensitivityDisplay(float NormalizedValue) const;

private:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> Slider_MouseSensitivity;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Progress_MouseSensitivity;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_MouseSensitivityValue;

    float PendingMouseSensitivityNormalized = 0.5f;
    bool bIsRefreshing = false;
};
