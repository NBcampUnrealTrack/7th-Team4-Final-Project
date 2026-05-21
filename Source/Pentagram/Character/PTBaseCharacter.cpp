#include "Character/PTBaseCharacter.h"

APTBaseCharacter::APTBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}
void APTBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

}
void APTBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void APTBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

