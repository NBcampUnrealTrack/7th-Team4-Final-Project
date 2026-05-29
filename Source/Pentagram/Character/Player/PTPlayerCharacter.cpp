#include "Character/Player/PTPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Skill/PTSkillComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

APTPlayerCharacter::APTPlayerCharacter()
{
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->TargetArmLength = 1500.f;
    SpringArmComp->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));
    SpringArmComp->bUsePawnControlRotation = false;
    SpringArmComp->bInheritPitch = false;
    SpringArmComp->bInheritRoll = false;
    SpringArmComp->bInheritYaw = false;
    SpringArmComp->bEnableCameraLag = false;
    SpringArmComp->SocketOffset = FVector(0.f, 0.f, 200.f);

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
    CameraComp->bUsePawnControlRotation = false;

    SkillComp = CreateDefaultSubobject<UPTSkillComponent>(TEXT("Skill"));

    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void APTPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    UE_LOG(LogTemp, Warning, TEXT("PossessedBy Called"));

    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentHP = MaxHP;
        PS->MaxHP = MaxHP;
        PS->CurrentMP = MaxMP;
        PS->MaxMP = MaxMP;
    }
}

void APTPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter BeginPlay Called"));

    AController* CT = GetController();
    if (CT)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller: %s"), *CT->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller is null"));
    }

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(
            HPRegenTimerHandle,
            this,
            &APTPlayerCharacter::RegenHP,
            5.f,
            true
            );
    }
}

void APTPlayerCharacter::PlayAttackMontage()
{
    if (!AttackMontages.IsValidIndex(ComboIndex)) return;

    bIsAttacking = true;
    bCanCombo = false;

    PlayAnimMontage(AttackMontages[ComboIndex]);
    ComboIndex++;
}

void APTPlayerCharacter::Server_UseSkill_Implementation(FName SkillID)
{
    if (SkillComp)
    {
        SkillComp->TryActivateSkill(SkillID);
    }
}

void APTPlayerCharacter::RegenHP()
{
    if (!HasAuthority()) return;

    float RegenAmount = MaxHP * 0.01f;
    CurrentHP = FMath::Min(CurrentHP + RegenAmount, MaxHP);

    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentHP = CurrentHP;
    }
}

void APTPlayerCharacter::MoveAction(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("MoveAction Called"));

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("PC is null"));
        return;
    }

    FHitResult HitResult;
    PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    UE_LOG(LogTemp, Warning, TEXT("HitResult: %s"), HitResult.bBlockingHit ? TEXT("Hit") : TEXT("No Hit"));

    if (HitResult.bBlockingHit)
    {
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(PC, HitResult.Location);
    }
}

void APTPlayerCharacter::AttackAction(const FInputActionValue& Value)
{
    if (bIsAttacking)
    {
        if (bCanCombo)
        {
            bCanCombo = false;
            PlayAttackMontage();
        }
        return;
    }

    PlayAttackMontage();
}

void APTPlayerCharacter::SkillAction1(const FInputActionValue& Value)
{
    Server_UseSkill(SkillComp->GetSkillAtSlot(0));
}

void APTPlayerCharacter::SkillAction2(const FInputActionValue& Value)
{
    Server_UseSkill(SkillComp->GetSkillAtSlot(1));
}

void APTPlayerCharacter::SkillAction3(const FInputActionValue& Value)
{
    Server_UseSkill(SkillComp->GetSkillAtSlot(2));
}

void APTPlayerCharacter::SkillAction4(const FInputActionValue& Value)
{
    Server_UseSkill(SkillComp->GetSkillAtSlot(3));
}

void APTPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent Called"));

    if (!IMC_Default)
    {
        UE_LOG(LogTemp, Warning, TEXT("IMC_Default is null"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("IMC_Default is valid"));
    }
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(IMC_Default, 0);
            UE_LOG(LogTemp, Warning, TEXT("IMC Added Successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Subsystem is null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PC is null in SetupInput"));
    }

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Move)
        {
            UE_LOG(LogTemp, Warning, TEXT("IA_Move Binding"));
            EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APTPlayerCharacter::MoveAction);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("IA_Move is null in binding"));
        }
    }
}

void APTPlayerCharacter::OnDeath()
{
    Super::OnDeath();

    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(HPRegenTimerHandle);

    // 호승님이 경험치 차감 구현 후에 API연동

    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
        //나중에 재생 시간을 애니메이션에 맞춰야 할 수도
    }

    OnPlayerDied.Broadcast();
}

void APTPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTPlayerCharacter,MaxMP);
    DOREPLIFETIME(APTPlayerCharacter,CurrentMP);
}
