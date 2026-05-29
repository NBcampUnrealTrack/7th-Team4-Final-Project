#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PTQuestDataRow.generated.h"

USTRUCT(BlueprintType)
struct PENTAGRAM_API FPTQuestDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FName QuestID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FText QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Quest")
    FText Description;
};
