#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PTSaveSubsystem.h"
#include "PTSaveGame.generated.h"

UCLASS()
class PENTAGRAM_API UPTSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY()
    FPTPlayerSaveData SaveData;
};
