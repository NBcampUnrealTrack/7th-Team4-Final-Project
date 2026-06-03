#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PTInteractableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPTInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

class PENTAGRAM_API IPTInteractableInterface
{
    GENERATED_BODY()

public:
    // C++과 블루프린트 양쪽에서 사용할 상호작용 함수
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    void Interact(AActor* InteractorCharacter);
};
