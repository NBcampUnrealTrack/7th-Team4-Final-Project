#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PTAudioSettings.generated.h"

class USoundClass;
class USoundMix;

UENUM(BlueprintType)
enum class EPTAudioChannel : uint8
{
    Master  UMETA(DisplayName = "Master"),
    BGM     UMETA(DisplayName = "BGM"),
    SFX     UMETA(DisplayName = "SFX"),
    UI      UMETA(DisplayName = "UI"),
    Ambient UMETA(DisplayName = "Ambient"),
};

USTRUCT(BlueprintType)
struct PENTAGRAM_API FPTAudioVolumeState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PT|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Master = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PT|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BGM = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PT|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFX = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PT|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float UI = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PT|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Ambient = 1.f;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PT Audio Settings"))
class PENTAGRAM_API UPTAudioSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const override { return TEXT("Game"); }

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio")
    TSoftObjectPtr<USoundMix> DefaultSoundMix;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio|SoundClass")
    TSoftObjectPtr<USoundClass> MasterSoundClass;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio|SoundClass")
    TSoftObjectPtr<USoundClass> BGMSoundClass;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio|SoundClass")
    TSoftObjectPtr<USoundClass> SFXSoundClass;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio|SoundClass")
    TSoftObjectPtr<USoundClass> UISoundClass;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio|SoundClass")
    TSoftObjectPtr<USoundClass> AmbientSoundClass;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio")
    FPTAudioVolumeState DefaultVolumes;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio", meta = (ClampMin = "0.0"))
    float VolumeFadeTime = 0.1f;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Audio")
    bool bApplyToChildSoundClasses = true;
};
