#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PTBaseCharacter.generated.h"

UCLASS()
class PENTAGRAM_API APTBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APTBaseCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
