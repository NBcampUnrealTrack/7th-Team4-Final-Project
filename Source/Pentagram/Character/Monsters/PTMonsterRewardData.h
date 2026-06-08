#pragma once

#include "CoreMinimal.h"
#include "PTMonsterRewardData.generated.h"

class AActor;
class APTGoldPickup;

USTRUCT(BlueprintType)
struct FPTMonsterRewardData
{
    GENERATED_BODY();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Reward")
    int32 RewardExp = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Reward")
    int32 GoldDropMin = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Reward")
    int32 GoldDropMax = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Reward")
    float EquipDropRate = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Reward")
    TSubclassOf<APTGoldPickup> GoldPickupClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Reward")
    TSubclassOf<AActor> EquipmentDropClass;
};
