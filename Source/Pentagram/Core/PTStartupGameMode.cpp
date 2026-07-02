#include "Core/PTStartupGameMode.h"

#include "Core/Subsystems/PTLoadingSubsystem.h"

APTStartupGameMode::APTStartupGameMode()
{
    DefaultPawnClass = nullptr;
    HUDClass = nullptr;
}

void APTStartupGameMode::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GameInstance = GetGameInstance();
    UPTLoadingSubsystem* LoadingSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTLoadingSubsystem>() : nullptr;
    if (LoadingSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Startup] PTLoadingSubsystem is not available."));
        return;
    }

    LoadingSubsystem->PreloadForStartup(
        TargetLevelName,
        MinDisplaySeconds,
        AdditionalPreloadAssets,
        AdditionalPreloadClasses);
}
