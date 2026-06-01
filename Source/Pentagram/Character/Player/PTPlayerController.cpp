#include "Character/Player/PTPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PTPlayerCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

APTPlayerController::APTPlayerController()
{
    PrimaryActorTick.bCanEverTick = false;
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
}

void APTPlayerController::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("Controller BeginPlay Called"));

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
    UE_LOG(LogTemp, Warning, TEXT("SetInputMode Called"));
}

void APTPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);
    UE_LOG(LogTemp, Warning, TEXT("AcknowledgePossession Called"));

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (IMC_Default)
        {
            Subsystem->AddMappingContext(IMC_Default, 0);
            UE_LOG(LogTemp, Warning, TEXT("IMC Added"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("IMC_Default is null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Subsystem is null"));
    }
}

void APTPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    UE_LOG(LogTemp, Warning, TEXT("Controller SetupInputComponent Called"));

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller EnhancedInput Cast Success"));
        if (IA_Move)
        {
            UE_LOG(LogTemp, Warning, TEXT("IA_Move Binding"));
            EnhancedInput->BindAction(IA_Move, ETriggerEvent::Started, this, &APTPlayerController::OnRightClick);
        }
        if (IA_Attack)
        {
            UE_LOG(LogTemp, Warning, TEXT("IA_Attack Binding"));
            EnhancedInput->BindAction(IA_Attack, ETriggerEvent::Started, this, &APTPlayerController::OnLeftClick);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("IA_Move is null"));
        }
    }
}

void APTPlayerController::PlayAttackMontage()
{
    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;
    if (!PlayerCharacter->AttackMontages.IsValidIndex(PlayerCharacter->ComboIndex)) return;

    PlayerCharacter->bIsAttacking = true;
    PlayerCharacter->bCanCombo = false;
    PlayerCharacter->PlayAnimMontage(PlayerCharacter->AttackMontages[PlayerCharacter->ComboIndex]);
    PlayerCharacter->ComboIndex++;
}

void APTPlayerController::OnRightClick(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("OnRightClick Called"));

    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    if (HitResult.bBlockingHit)
    {
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.Location);
    }
}

void APTPlayerController::OnLeftClick(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("OnLeftClick Called"));

    APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;

    if (PlayerCharacter->bIsAttacking)
    {
        if (PlayerCharacter->bCanCombo)
        {
            PlayerCharacter->bCanCombo = false;
            PlayAttackMontage();
        }
        return;
    }
    PlayAttackMontage();
}
