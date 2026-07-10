#include "Audio/PTAudioSubsystem.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

namespace
{
float ClampVolume(float Volume)
{
    return FMath::Clamp(Volume, 0.f, 1.f);
}
}

void UPTAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ApplyDefaultAudioSettings();
}

void UPTAudioSubsystem::Deinitialize()
{
    PopDefaultSoundMix();

    Super::Deinitialize();
}

void UPTAudioSubsystem::ApplyDefaultAudioSettings()
{
    const UPTAudioSettings* AudioSettings = GetDefault<UPTAudioSettings>();
    if (AudioSettings == nullptr)
    {
        return;
    }

    ApplyAudioSettings(AudioSettings->DefaultVolumes);
}

void UPTAudioSubsystem::ApplyAudioSettings(const FPTAudioVolumeState& InVolumeState)
{
    CurrentVolumeState.Master = ClampVolume(InVolumeState.Master);
    CurrentVolumeState.BGM = ClampVolume(InVolumeState.BGM);
    CurrentVolumeState.SFX = ClampVolume(InVolumeState.SFX);
    CurrentVolumeState.UI = ClampVolume(InVolumeState.UI);
    CurrentVolumeState.Ambient = ClampVolume(InVolumeState.Ambient);

    PushDefaultSoundMix();

    ApplyChannelVolume(EPTAudioChannel::Master, CurrentVolumeState.Master);
    ApplyChannelVolume(EPTAudioChannel::BGM, CurrentVolumeState.BGM);
    ApplyChannelVolume(EPTAudioChannel::SFX, CurrentVolumeState.SFX);
    ApplyChannelVolume(EPTAudioChannel::UI, CurrentVolumeState.UI);
    ApplyChannelVolume(EPTAudioChannel::Ambient, CurrentVolumeState.Ambient);
}

void UPTAudioSubsystem::SetChannelVolume(EPTAudioChannel Channel, float Volume)
{
    SetStoredChannelVolume(Channel, Volume);
    PushDefaultSoundMix();
    ApplyChannelVolume(Channel, GetChannelVolume(Channel));
}

void UPTAudioSubsystem::SetMasterVolume(float Volume)
{
    SetChannelVolume(EPTAudioChannel::Master, Volume);
}

void UPTAudioSubsystem::SetBGMVolume(float Volume)
{
    SetChannelVolume(EPTAudioChannel::BGM, Volume);
}

void UPTAudioSubsystem::SetSFXVolume(float Volume)
{
    SetChannelVolume(EPTAudioChannel::SFX, Volume);
}

void UPTAudioSubsystem::SetUIVolume(float Volume)
{
    SetChannelVolume(EPTAudioChannel::UI, Volume);
}

void UPTAudioSubsystem::SetAmbientVolume(float Volume)
{
    SetChannelVolume(EPTAudioChannel::Ambient, Volume);
}

float UPTAudioSubsystem::GetChannelVolume(EPTAudioChannel Channel) const
{
    switch (Channel)
    {
    case EPTAudioChannel::Master:
        return CurrentVolumeState.Master;
    case EPTAudioChannel::BGM:
        return CurrentVolumeState.BGM;
    case EPTAudioChannel::SFX:
        return CurrentVolumeState.SFX;
    case EPTAudioChannel::UI:
        return CurrentVolumeState.UI;
    case EPTAudioChannel::Ambient:
        return CurrentVolumeState.Ambient;
    default:
        return 1.f;
    }
}

void UPTAudioSubsystem::PushDefaultSoundMix()
{
    if (bDefaultSoundMixPushed)
    {
        return;
    }

    USoundMix* SoundMix = ResolveDefaultSoundMix();
    UWorld* World = GetWorld();
    if (SoundMix == nullptr || World == nullptr)
    {
        return;
    }

    UGameplayStatics::PushSoundMixModifier(World, SoundMix);
    bDefaultSoundMixPushed = true;
}

void UPTAudioSubsystem::PopDefaultSoundMix()
{
    if (!bDefaultSoundMixPushed)
    {
        return;
    }

    USoundMix* SoundMix = ResolveDefaultSoundMix();
    UWorld* World = GetWorld();
    if (SoundMix != nullptr && World != nullptr)
    {
        UGameplayStatics::PopSoundMixModifier(World, SoundMix);
    }

    bDefaultSoundMixPushed = false;
}

void UPTAudioSubsystem::ApplyChannelVolume(EPTAudioChannel Channel, float Volume)
{
    USoundMix* SoundMix = ResolveDefaultSoundMix();
    USoundClass* SoundClass = ResolveSoundClass(Channel);
    UWorld* World = GetWorld();
    if (SoundMix == nullptr || SoundClass == nullptr || World == nullptr)
    {
        return;
    }

    const UPTAudioSettings* AudioSettings = GetDefault<UPTAudioSettings>();
    const float FadeTime = AudioSettings != nullptr ? AudioSettings->VolumeFadeTime : 0.f;
    const bool bApplyToChildren = AudioSettings != nullptr ? AudioSettings->bApplyToChildSoundClasses : true;

    UGameplayStatics::SetSoundMixClassOverride(
        World,
        SoundMix,
        SoundClass,
        ClampVolume(Volume),
        1.f,
        FadeTime,
        bApplyToChildren);
}

USoundMix* UPTAudioSubsystem::ResolveDefaultSoundMix() const
{
    const UPTAudioSettings* AudioSettings = GetDefault<UPTAudioSettings>();
    if (AudioSettings == nullptr || AudioSettings->DefaultSoundMix.IsNull())
    {
        return nullptr;
    }

    return AudioSettings->DefaultSoundMix.LoadSynchronous();
}

USoundClass* UPTAudioSubsystem::ResolveSoundClass(EPTAudioChannel Channel) const
{
    const UPTAudioSettings* AudioSettings = GetDefault<UPTAudioSettings>();
    if (AudioSettings == nullptr)
    {
        return nullptr;
    }

    switch (Channel)
    {
    case EPTAudioChannel::Master:
        return AudioSettings->MasterSoundClass.LoadSynchronous();
    case EPTAudioChannel::BGM:
        return AudioSettings->BGMSoundClass.LoadSynchronous();
    case EPTAudioChannel::SFX:
        return AudioSettings->SFXSoundClass.LoadSynchronous();
    case EPTAudioChannel::UI:
        return AudioSettings->UISoundClass.LoadSynchronous();
    case EPTAudioChannel::Ambient:
        return AudioSettings->AmbientSoundClass.LoadSynchronous();
    default:
        return nullptr;
    }
}

void UPTAudioSubsystem::SetStoredChannelVolume(EPTAudioChannel Channel, float Volume)
{
    const float ClampedVolume = ClampVolume(Volume);

    switch (Channel)
    {
    case EPTAudioChannel::Master:
        CurrentVolumeState.Master = ClampedVolume;
        break;
    case EPTAudioChannel::BGM:
        CurrentVolumeState.BGM = ClampedVolume;
        break;
    case EPTAudioChannel::SFX:
        CurrentVolumeState.SFX = ClampedVolume;
        break;
    case EPTAudioChannel::UI:
        CurrentVolumeState.UI = ClampedVolume;
        break;
    case EPTAudioChannel::Ambient:
        CurrentVolumeState.Ambient = ClampedVolume;
        break;
    default:
        break;
    }
}
