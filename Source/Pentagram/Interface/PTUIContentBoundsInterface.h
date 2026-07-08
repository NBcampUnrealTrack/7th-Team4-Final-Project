#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PTUIContentBoundsInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPTUIContentBoundsInterface : public UInterface
{
    GENERATED_BODY()
};

class PENTAGRAM_API IPTUIContentBoundsInterface
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PT|UI")
    bool IsScreenPositionOverContent(const FVector2D& ScreenPosition) const;
};
