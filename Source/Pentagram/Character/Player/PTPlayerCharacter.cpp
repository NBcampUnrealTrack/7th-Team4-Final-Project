#include "Character/Player/PTPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "PTBasePlayerState.h"
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

    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void APTPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

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
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    FHitResult HitResult;
    PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

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

void APTPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APTPlayerCharacter::MoveAction);
        EnhancedInput->BindAction(IA_Attack, ETriggerEvent::Triggered, this, &APTPlayerCharacter::AttackAction);
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
