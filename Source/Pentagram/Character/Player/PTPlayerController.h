#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "PTPlayerController.generated.h"


UCLASS()
class PENTAGRAM_API APTPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    APTPlayerController();

   /* UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<class UInputMappingContext> IMC_Default;*/

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<class UInputAction> IA_Move;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Attack;

protected:
    void PlayAttackMontage();
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void AcknowledgePossession(class APawn* P) override;

private:
    void OnRightClick(const FInputActionValue& Value);
    void OnLeftClick(const FInputActionValue& Value);
};
