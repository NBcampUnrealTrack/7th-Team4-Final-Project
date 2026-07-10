#include "UI/Widget/Setting/PTControlSettingsWidget.h"

#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Input/PTControlSettingsSubsystem.h"

void UPTControlSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Slider_MouseSensitivity != nullptr)
    {
        Slider_MouseSensitivity->OnValueChanged.AddDynamic(this, &UPTControlSettingsWidget::HandleMouseSensitivityChanged);
    }

    RefreshFromControlSettings();
}

void UPTControlSettingsWidget::NativeDestruct()
{
    if (Slider_MouseSensitivity != nullptr)
    {
        Slider_MouseSensitivity->OnValueChanged.RemoveDynamic(this, &UPTControlSettingsWidget::HandleMouseSensitivityChanged);
    }

    Super::NativeDestruct();
}

void UPTControlSettingsWidget::HandleMouseSensitivityChanged(float Value)
{
    if (bIsRefreshing)
    {
        return;
    }

    const float QuantizedValue = QuantizeNormalizedValue(Value);

    bIsRefreshing = true;
    if (Slider_MouseSensitivity != nullptr)
    {
        Slider_MouseSensitivity->SetValue(QuantizedValue);
    }
    bIsRefreshing = false;

    PendingMouseSensitivityNormalized = QuantizedValue;
    SetMouseSensitivityDisplay(QuantizedValue);
}

void UPTControlSettingsWidget::ApplySettings()
{
    UPTControlSettingsSubsystem* ControlSettingsSubsystem = ResolveControlSettingsSubsystem();
    if (ControlSettingsSubsystem == nullptr)
    {
        return;
    }

    ControlSettingsSubsystem->SetMouseSensitivityNormalized(PendingMouseSensitivityNormalized);
    RefreshFromControlSettings();
}

void UPTControlSettingsWidget::ResetSettingsToDefaults()
{
    UPTControlSettingsSubsystem* ControlSettingsSubsystem = ResolveControlSettingsSubsystem();
    if (ControlSettingsSubsystem == nullptr)
    {
        return;
    }

    PendingMouseSensitivityNormalized = QuantizeNormalizedValue(
        UPTControlSettingsSubsystem::NormalizeMouseSensitivity(
            UPTControlSettingsSubsystem::GetDefaultMouseSensitivity()));

    bIsRefreshing = true;
    if (Slider_MouseSensitivity != nullptr)
    {
        Slider_MouseSensitivity->SetValue(PendingMouseSensitivityNormalized);
    }
    bIsRefreshing = false;

    SetMouseSensitivityDisplay(PendingMouseSensitivityNormalized);
}

UPTControlSettingsSubsystem* UPTControlSettingsWidget::ResolveControlSettingsSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UPTControlSettingsSubsystem>() : nullptr;
}

void UPTControlSettingsWidget::RefreshFromControlSettings()
{
    UPTControlSettingsSubsystem* ControlSettingsSubsystem = ResolveControlSettingsSubsystem();
    if (ControlSettingsSubsystem == nullptr)
    {
        return;
    }

    const float NormalizedValue = QuantizeNormalizedValue(ControlSettingsSubsystem->GetMouseSensitivityNormalized());
    PendingMouseSensitivityNormalized = NormalizedValue;

    bIsRefreshing = true;
    if (Slider_MouseSensitivity != nullptr)
    {
        Slider_MouseSensitivity->SetValue(NormalizedValue);
    }
    bIsRefreshing = false;

    SetMouseSensitivityDisplay(NormalizedValue);
}

float UPTControlSettingsWidget::QuantizeNormalizedValue(float Value) const
{
    const int32 DisplayValue = FMath::Clamp(FMath::RoundToInt(FMath::Clamp(Value, 0.f, 1.f) * 99.f) + 1, 1, 100);
    return static_cast<float>(DisplayValue - 1) / 99.f;
}

void UPTControlSettingsWidget::SetMouseSensitivityDisplay(float NormalizedValue) const
{
    const float ClampedValue = FMath::Clamp(NormalizedValue, 0.f, 1.f);

    if (Progress_MouseSensitivity != nullptr)
    {
        Progress_MouseSensitivity->SetPercent(ClampedValue);
    }

    if (Text_MouseSensitivityValue != nullptr)
    {
        const int32 DisplayValue = FMath::Clamp(FMath::RoundToInt(ClampedValue * 99.f) + 1, 1, 100);
        Text_MouseSensitivityValue->SetText(FText::AsNumber(DisplayValue));
    }
}
