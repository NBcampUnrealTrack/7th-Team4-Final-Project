#include "Input/PTControlSettingsSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
const TCHAR* ControlSettingsSection = TEXT("/Script/Pentagram.PTControlSettings");
const TCHAR* MouseSensitivityKey = TEXT("MouseSensitivity");
}

void UPTControlSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LoadControlSettings();
}

void UPTControlSettingsSubsystem::SetMouseSensitivity(float InMouseSensitivity)
{
    MouseSensitivity = FMath::Clamp(InMouseSensitivity, MinMouseSensitivity, MaxMouseSensitivity);
    SaveControlSettings();
    ApplyControlSettingsToLocalPlayers();
}

void UPTControlSettingsSubsystem::SetMouseSensitivityNormalized(float NormalizedValue)
{
    SetMouseSensitivity(DenormalizeMouseSensitivity(NormalizedValue));
}

void UPTControlSettingsSubsystem::ApplyControlSettings(APlayerController* PlayerController) const
{
    if (PlayerController == nullptr || PlayerController->PlayerInput == nullptr)
    {
        return;
    }

    PlayerController->PlayerInput->SetMouseSensitivity(MouseSensitivity);
}

void UPTControlSettingsSubsystem::ApplyControlSettingsToLocalPlayers() const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        if (PlayerController != nullptr && PlayerController->IsLocalController())
        {
            ApplyControlSettings(PlayerController);
        }
    }
}

void UPTControlSettingsSubsystem::ResetControlSettings()
{
    SetMouseSensitivity(DefaultMouseSensitivity);
}

float UPTControlSettingsSubsystem::GetMouseSensitivityNormalized() const
{
    return NormalizeMouseSensitivity(MouseSensitivity);
}

int32 UPTControlSettingsSubsystem::GetMouseSensitivityDisplayValue() const
{
    return MouseSensitivityToDisplayValue(MouseSensitivity);
}

float UPTControlSettingsSubsystem::NormalizeMouseSensitivity(float InMouseSensitivity)
{
    const float ClampedSensitivity = FMath::Clamp(InMouseSensitivity, MinMouseSensitivity, MaxMouseSensitivity);
    return (ClampedSensitivity - MinMouseSensitivity) / (MaxMouseSensitivity - MinMouseSensitivity);
}

float UPTControlSettingsSubsystem::DenormalizeMouseSensitivity(float NormalizedValue)
{
    const float ClampedNormalizedValue = FMath::Clamp(NormalizedValue, 0.f, 1.f);
    return FMath::Lerp(MinMouseSensitivity, MaxMouseSensitivity, ClampedNormalizedValue);
}

int32 UPTControlSettingsSubsystem::MouseSensitivityToDisplayValue(float InMouseSensitivity)
{
    const float NormalizedValue = NormalizeMouseSensitivity(InMouseSensitivity);
    return FMath::Clamp(FMath::RoundToInt(NormalizedValue * 99.f) + 1, 1, 100);
}

float UPTControlSettingsSubsystem::DisplayValueToMouseSensitivity(int32 DisplayValue)
{
    const int32 ClampedDisplayValue = FMath::Clamp(DisplayValue, 1, 100);
    return DenormalizeMouseSensitivity(static_cast<float>(ClampedDisplayValue - 1) / 99.f);
}

void UPTControlSettingsSubsystem::LoadControlSettings()
{
    float LoadedMouseSensitivity = DefaultMouseSensitivity;

    if (GConfig != nullptr)
    {
        GConfig->GetFloat(ControlSettingsSection, MouseSensitivityKey, LoadedMouseSensitivity, GGameUserSettingsIni);
    }

    MouseSensitivity = FMath::Clamp(LoadedMouseSensitivity, MinMouseSensitivity, MaxMouseSensitivity);
}

void UPTControlSettingsSubsystem::SaveControlSettings() const
{
    if (GConfig == nullptr)
    {
        return;
    }

    GConfig->SetFloat(ControlSettingsSection, MouseSensitivityKey, MouseSensitivity, GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}
