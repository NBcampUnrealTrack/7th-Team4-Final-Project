#pragma once

#include "CoreMinimal.h"
#include "Audio/PTAudioSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTAudioSubsystem.generated.h"

class USoundClass;
class USoundMix;

UCLASS()
class PENTAGRAM_API UPTAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "PT|Audio")
    void ApplyDefaultAudioSettings();

    UFUNCTION(BlueprintCallable, Category = "PT|Audio")
    void ApplyAudioSettings(const FPTAudioVolumeState& InVolumeState);

    UFUNCTION(BlueprintCallable, Category = "PT|Audio")
    void SetChannelVolume(EPTAudioChannel Channel, float Volume);

    UFUNCTION(BlueprintCallable, Category = "PT|Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "PT|Audio")
    void SetBGMVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "PT|Audio")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "PT|Audio")
    void SetUIVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "PT|Audio")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "PT|Audio")
    float GetChannelVolume(EPTAudioChannel Channel) const;

    UFUNCTION(BlueprintPure, Category = "PT|Audio")
    FPTAudioVolumeState GetVolumeState() const { return CurrentVolumeState; }

private:
    void PushDefaultSoundMix();
    void PopDefaultSoundMix();
    void ApplyChannelVolume(EPTAudioChannel Channel, float Volume);
    USoundMix* ResolveDefaultSoundMix() const;
    USoundClass* ResolveSoundClass(EPTAudioChannel Channel) const;
    void SetStoredChannelVolume(EPTAudioChannel Channel, float Volume);

private:
    FPTAudioVolumeState CurrentVolumeState;
    bool bDefaultSoundMixPushed = false;
};
