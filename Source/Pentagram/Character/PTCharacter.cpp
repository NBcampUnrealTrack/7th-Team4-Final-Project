#include "Character/PTCharacter.h"

APTCharacter::APTCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}
void APTCharacter::BeginPlay()
{
	Super::BeginPlay();

}
void APTCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void APTCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

