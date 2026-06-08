#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PTLevelDataRow.generated.h"

USTRUCT(BlueprintType)
struct PENTAGRAM_API FPTLevelDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Level", meta = (ClampMin = "1"))
    int32 PlayerLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Level", meta = (ClampMin = "1"))
    int32 RequiredExp = 100;
};
