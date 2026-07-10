#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTControlSettingsSubsystem.generated.h"

class APlayerController;

UCLASS()
class PENTAGRAM_API UPTControlSettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "PT|Control Settings")
    void SetMouseSensitivity(float InMouseSensitivity);

    UFUNCTION(BlueprintCallable, Category = "PT|Control Settings")
    void SetMouseSensitivityNormalized(float NormalizedValue);

    UFUNCTION(BlueprintCallable, Category = "PT|Control Settings")
    void ApplyControlSettings(APlayerController* PlayerController) const;

    UFUNCTION(BlueprintCallable, Category = "PT|Control Settings")
    void ApplyControlSettingsToLocalPlayers() const;

    UFUNCTION(BlueprintCallable, Category = "PT|Control Settings")
    void ResetControlSettings();

    UFUNCTION(BlueprintPure, Category = "PT|Control Settings")
    float GetMouseSensitivity() const { return MouseSensitivity; }

    UFUNCTION(BlueprintPure, Category = "PT|Control Settings")
    float GetMouseSensitivityNormalized() const;

    UFUNCTION(BlueprintPure, Category = "PT|Control Settings")
    int32 GetMouseSensitivityDisplayValue() const;

    UFUNCTION(BlueprintPure, Category = "PT|Control Settings")
    static float GetDefaultMouseSensitivity() { return DefaultMouseSensitivity; }

    UFUNCTION(BlueprintPure, Category = "PT|Control Settings")
    static float GetMinMouseSensitivity() { return MinMouseSensitivity; }

    UFUNCTION(BlueprintPure, Category = "PT|Control Settings")
    static float GetMaxMouseSensitivity() { return MaxMouseSensitivity; }

    static float NormalizeMouseSensitivity(float InMouseSensitivity);
    static float DenormalizeMouseSensitivity(float NormalizedValue);
    static int32 MouseSensitivityToDisplayValue(float InMouseSensitivity);
    static float DisplayValueToMouseSensitivity(int32 DisplayValue);

private:
    void LoadControlSettings();
    void SaveControlSettings() const;

private:
    static constexpr float MinMouseSensitivity = 0.02f;
    static constexpr float MaxMouseSensitivity = 2.0f;
    static constexpr float DefaultMouseSensitivity = 1.0f;

    float MouseSensitivity = DefaultMouseSensitivity;
};
