#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PTCharacter.generated.h"

UCLASS()
class PENTAGRAM_API APTCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APTCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
