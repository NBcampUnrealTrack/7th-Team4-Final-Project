#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTStartupGameMode.generated.h"

UCLASS()
class PENTAGRAM_API APTStartupGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APTStartupGameMode();

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(EditAnywhere, Category = "PT|Startup")
    FName TargetLevelName = TEXT("L_Intro");

    UPROPERTY(EditAnywhere, Category = "PT|Startup", meta = (ClampMin = "0.0"))
    float MinDisplaySeconds = 0.75f;

    UPROPERTY(EditAnywhere, Category = "PT|Startup")
    TArray<TSoftObjectPtr<UObject>> AdditionalPreloadAssets;

    UPROPERTY(EditAnywhere, Category = "PT|Startup")
    TArray<TSoftClassPtr<UObject>> AdditionalPreloadClasses;
};
