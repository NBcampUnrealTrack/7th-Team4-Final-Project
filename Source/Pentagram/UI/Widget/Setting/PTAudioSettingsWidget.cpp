#include "UI/Widget/Setting/PTAudioSettingsWidget.h"

#include "Audio/PTAudioSubsystem.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

void UPTAudioSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Slider_Master != nullptr)
    {
        Slider_Master->OnValueChanged.AddDynamic(this, &UPTAudioSettingsWidget::HandleMasterVolumeChanged);
    }

    if (Slider_BGM != nullptr)
    {
        Slider_BGM->OnValueChanged.AddDynamic(this, &UPTAudioSettingsWidget::HandleBGMVolumeChanged);
    }

    if (Slider_SFX != nullptr)
    {
        Slider_SFX->OnValueChanged.AddDynamic(this, &UPTAudioSettingsWidget::HandleSFXVolumeChanged);
    }

    if (Slider_UI != nullptr)
    {
        Slider_UI->OnValueChanged.AddDynamic(this, &UPTAudioSettingsWidget::HandleUIVolumeChanged);
    }

    if (Slider_Ambient != nullptr)
    {
        Slider_Ambient->OnValueChanged.AddDynamic(this, &UPTAudioSettingsWidget::HandleAmbientVolumeChanged);
    }

    RefreshFromAudioSubsystem();
}

void UPTAudioSettingsWidget::NativeDestruct()
{
    if (Slider_Master != nullptr)
    {
        Slider_Master->OnValueChanged.RemoveDynamic(this, &UPTAudioSettingsWidget::HandleMasterVolumeChanged);
    }

    if (Slider_BGM != nullptr)
    {
        Slider_BGM->OnValueChanged.RemoveDynamic(this, &UPTAudioSettingsWidget::HandleBGMVolumeChanged);
    }

    if (Slider_SFX != nullptr)
    {
        Slider_SFX->OnValueChanged.RemoveDynamic(this, &UPTAudioSettingsWidget::HandleSFXVolumeChanged);
    }

    if (Slider_UI != nullptr)
    {
        Slider_UI->OnValueChanged.RemoveDynamic(this, &UPTAudioSettingsWidget::HandleUIVolumeChanged);
    }

    if (Slider_Ambient != nullptr)
    {
        Slider_Ambient->OnValueChanged.RemoveDynamic(this, &UPTAudioSettingsWidget::HandleAmbientVolumeChanged);
    }

    Super::NativeDestruct();
}

void UPTAudioSettingsWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    RefreshFromAudioSubsystem();
}

void UPTAudioSettingsWidget::HandleMasterVolumeChanged(float Value)
{
    if (bIsRefreshing)
    {
        return;
    }

    SetChannelVolumeFromSlider(EPTAudioChannel::Master, Slider_Master, Value);
}

void UPTAudioSettingsWidget::HandleBGMVolumeChanged(float Value)
{
    if (bIsRefreshing)
    {
        return;
    }

    SetChannelVolumeFromSlider(EPTAudioChannel::BGM, Slider_BGM, Value);
}

void UPTAudioSettingsWidget::HandleSFXVolumeChanged(float Value)
{
    if (bIsRefreshing)
    {
        return;
    }

    SetChannelVolumeFromSlider(EPTAudioChannel::SFX, Slider_SFX, Value);
}

void UPTAudioSettingsWidget::HandleUIVolumeChanged(float Value)
{
    if (bIsRefreshing)
    {
        return;
    }

    SetChannelVolumeFromSlider(EPTAudioChannel::UI, Slider_UI, Value);
}

void UPTAudioSettingsWidget::HandleAmbientVolumeChanged(float Value)
{
    if (bIsRefreshing)
    {
        return;
    }

    SetChannelVolumeFromSlider(EPTAudioChannel::Ambient, Slider_Ambient, Value);
}

void UPTAudioSettingsWidget::ApplySettings()
{
    UPTAudioSubsystem* AudioSubsystem = ResolveAudioSubsystem();
    if (AudioSubsystem == nullptr)
    {
        return;
    }

    AudioSubsystem->ApplyAudioSettings(AudioSubsystem->GetVolumeState());
    RefreshFromAudioSubsystem();
}

void UPTAudioSettingsWidget::ResetSettingsToDefaults()
{
    UPTAudioSubsystem* AudioSubsystem = ResolveAudioSubsystem();
    if (AudioSubsystem == nullptr)
    {
        return;
    }

    AudioSubsystem->ApplyDefaultAudioSettings();
    RefreshFromAudioSubsystem();
}

UPTAudioSubsystem* UPTAudioSettingsWidget::ResolveAudioSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UPTAudioSubsystem>() : nullptr;
}

float UPTAudioSettingsWidget::QuantizeVolume(float Value) const
{
    return FMath::Clamp(FMath::RoundToFloat(Value * 100.f) / 100.f, 0.f, 1.f);
}

void UPTAudioSettingsWidget::SetChannelVolumeFromSlider(EPTAudioChannel Channel, USlider* Slider, float Value)
{
    const float QuantizedValue = QuantizeVolume(Value);

    bIsRefreshing = true;
    SetSliderValue(Slider, QuantizedValue);
    bIsRefreshing = false;

    if (UPTAudioSubsystem* AudioSubsystem = ResolveAudioSubsystem())
    {
        AudioSubsystem->SetChannelVolume(Channel, QuantizedValue);
    }

    UpdateChannelDisplay(Channel, QuantizedValue);
}

void UPTAudioSettingsWidget::RefreshFromAudioSubsystem()
{
    UPTAudioSubsystem* AudioSubsystem = ResolveAudioSubsystem();
    if (AudioSubsystem == nullptr)
    {
        return;
    }

    FPTAudioVolumeState VolumeState = AudioSubsystem->GetVolumeState();
    VolumeState.Master = QuantizeVolume(VolumeState.Master);
    VolumeState.BGM = QuantizeVolume(VolumeState.BGM);
    VolumeState.SFX = QuantizeVolume(VolumeState.SFX);
    VolumeState.UI = QuantizeVolume(VolumeState.UI);
    VolumeState.Ambient = QuantizeVolume(VolumeState.Ambient);
    bIsRefreshing = true;
    SetSliderValue(Slider_Master, VolumeState.Master);
    SetSliderValue(Slider_BGM, VolumeState.BGM);
    SetSliderValue(Slider_SFX, VolumeState.SFX);
    SetSliderValue(Slider_UI, VolumeState.UI);
    SetSliderValue(Slider_Ambient, VolumeState.Ambient);
    bIsRefreshing = false;

    SetValueText(Text_MasterValue, VolumeState.Master);
    SetValueText(Text_BGMValue, VolumeState.BGM);
    SetValueText(Text_SFXValue, VolumeState.SFX);
    SetValueText(Text_UIValue, VolumeState.UI);
    SetValueText(Text_AmbientValue, VolumeState.Ambient);

    SetProgressValue(Progress_Master, VolumeState.Master);
    SetProgressValue(Progress_BGM, VolumeState.BGM);
    SetProgressValue(Progress_SFX, VolumeState.SFX);
    SetProgressValue(Progress_UI, VolumeState.UI);
    SetProgressValue(Progress_Ambient, VolumeState.Ambient);
}

void UPTAudioSettingsWidget::SetSliderValue(USlider* Slider, float Value) const
{
    if (Slider != nullptr)
    {
        Slider->SetValue(FMath::Clamp(Value, 0.f, 1.f));
    }
}

void UPTAudioSettingsWidget::SetProgressValue(UProgressBar* ProgressBar, float Value) const
{
    if (ProgressBar != nullptr)
    {
        ProgressBar->SetPercent(FMath::Clamp(Value, 0.f, 1.f));
    }
}

void UPTAudioSettingsWidget::SetValueText(UTextBlock* TextBlock, float Value) const
{
    if (TextBlock == nullptr)
    {
        return;
    }

    const int32 PercentValue = FMath::RoundToInt(FMath::Clamp(Value, 0.f, 1.f) * 100.f);
    TextBlock->SetText(FText::AsNumber(PercentValue));
}

void UPTAudioSettingsWidget::UpdateChannelDisplay(EPTAudioChannel Channel, float Value) const
{
    switch (Channel)
    {
    case EPTAudioChannel::Master:
        SetValueText(Text_MasterValue, Value);
        SetProgressValue(Progress_Master, Value);
        break;
    case EPTAudioChannel::BGM:
        SetValueText(Text_BGMValue, Value);
        SetProgressValue(Progress_BGM, Value);
        break;
    case EPTAudioChannel::SFX:
        SetValueText(Text_SFXValue, Value);
        SetProgressValue(Progress_SFX, Value);
        break;
    case EPTAudioChannel::UI:
        SetValueText(Text_UIValue, Value);
        SetProgressValue(Progress_UI, Value);
        break;
    case EPTAudioChannel::Ambient:
        SetValueText(Text_AmbientValue, Value);
        SetProgressValue(Progress_Ambient, Value);
        break;
    default:
        break;
    }
}
