#include "Character/Player/PTPlayerController.h"

#include "EnhancedInputSubsystems.h"

APTPlayerController::APTPlayerController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APTPlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true;
    
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(IMC_Default, 0);
    }

}
