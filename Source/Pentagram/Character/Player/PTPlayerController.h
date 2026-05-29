#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PTPlayerController.generated.h"


UCLASS()
class PENTAGRAM_API APTPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    APTPlayerController();

   /* UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<class UInputMappingContext> IMC_Default;*/

    virtual void BeginPlay() override;
    virtual void AcknowledgePossession(class APawn* P) override;
};
