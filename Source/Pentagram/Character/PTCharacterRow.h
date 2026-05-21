#pragma once

#include "Engine/DataTable.h"
#include "PTCharacterRow.generated.h"

USTRUCT(BlueprintType)
struct FPTCharacterRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString CharacterName;
};
