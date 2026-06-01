#include "Character/Player/PTPlayerController.h"

#include "CommonActivatableWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "PTPlayerCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "UI/Screens/LayOut/PTPrimaryLayout.h"
#include "Skill/PTSkillComponent.h"

APTPlayerController::APTPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
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

    AddUIInputMapping();

    if (PrimaryLayoutClass)
    {
        PrimaryLayout = CreateWidget<UPTPrimaryLayout>(this, PrimaryLayoutClass);
        if (PrimaryLayout)
        {
            PrimaryLayout->AddToPlayerScreen(0);
            if (ULocalPlayer* LP = GetLocalPlayer())
            {
                if (UPTUIManagerSubsystem* UIMgr = LP->GetSubsystem<UPTUIManagerSubsystem>())
                {
                    UIMgr->RegisterPrimaryLayout(PrimaryLayout);
                    PushInitialHUD();
                }
            }
        }
    }


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

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (EnhancedInput)
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
        if (IA_Inventory)
        {
            EnhancedInput->BindAction(IA_Inventory, ETriggerEvent::Started, this, &APTPlayerController::OnInventoryPressed);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("IA_Move is null"));
        }
    }

    AddUIInputMapping();
}

void APTPlayerController::OnSkill1(const FInputActionValue& Value)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
        PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(0));
}

void APTPlayerController::OnSkill2(const FInputActionValue& Value)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
        PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(1));
}

void APTPlayerController::OnSkill3(const FInputActionValue& Value)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
        PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(2));
}

void APTPlayerController::OnSkill4(const FInputActionValue& Value)
{
    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetPawn()))
        PC->Server_UseSkill(PC->SkillComp->GetSkillAtSlot(3));
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

void APTPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveUIInputMapping();
    Super::EndPlay(EndPlayReason);
}

void APTPlayerController::OnInventoryPressed()
{
    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP)
    {
        return;
    }

    UPTUIManagerSubsystem* UI = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UI || !InventoryClass)
    {
        return;
    }

    UI->ToggleInventory(InventoryClass);
}

void APTPlayerController::PushInitialHUD()
{
    if (!InitialHUDClass)
    {
        return;
    }

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UPTUIManagerSubsystem* UIMgr = LP->GetSubsystem<UPTUIManagerSubsystem>())
        {
            UIMgr->PushWidget(InitialHUDClass, EPTUILayer::HUD);
        }
    }
}

void APTPlayerController::AddUIInputMapping()
{
    if (bUIInputMappingAdded || !IMC_UI)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!InputSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("EnhancedInputLocalPlayerSubsystem not found."));
        return;
    }

    InputSubsystem->AddMappingContext(IMC_UI, 100);
    bUIInputMappingAdded = true;

    UE_LOG(LogTemp, Warning, TEXT("UI Input Mapping Added: %s / Priority=100"), *IMC_UI->GetName());

}

void APTPlayerController::RemoveUIInputMapping()
{
    if (!bUIInputMappingAdded || !IMC_UI)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!InputSubsystem)
    {
        return;
    }

    InputSubsystem->RemoveMappingContext(IMC_UI);
    bUIInputMappingAdded = false;
}
