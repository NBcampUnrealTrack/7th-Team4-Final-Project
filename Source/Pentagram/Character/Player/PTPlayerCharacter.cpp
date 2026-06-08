#include "Character/Player/PTPlayerCharacter.h"

#include "PTPlayerController.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Skill/PTSkillComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Core/PTGameMode.h"
#include "Net/UnrealNetwork.h"
#include "PTInventoryComponent.h"
#include "PTEquipmentComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Core/Interface/PTInteractableInterface.h"

APTPlayerCharacter::APTPlayerCharacter()
{
    SetReplicateMovement(true);
    GetCharacterMovement()->SetIsReplicated(true);
    bReplicates = true;

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->TargetArmLength = 1500.f;
    SpringArmComp->SetRelativeRotation(FRotator(-55.f, 45.f, 0.f));
    SpringArmComp->bUsePawnControlRotation = false;
    SpringArmComp->bInheritPitch = false;
    SpringArmComp->bInheritRoll = false;
    SpringArmComp->bInheritYaw = false;
    SpringArmComp->bEnableCameraLag = false;
    SpringArmComp->bDoCollisionTest = false;
    SpringArmComp->SocketOffset = FVector(0.f, 0.f, 200.f);

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
    CameraComp->bUsePawnControlRotation = false;

    SkillComp          = CreateDefaultSubobject<UPTSkillComponent>(TEXT("Skill"));
    InventoryComponent = CreateDefaultSubobject<UPTInventoryComponent>(TEXT("InventoryComponent"));
    EquipmentComponent = CreateDefaultSubobject<UPTEquipmentComponent>(TEXT("EquipmentComponent"));

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
    bUseControllerRotationYaw = false;
}

void APTPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    UE_LOG(LogTemp, Warning, TEXT("PossessedBy Called"));

    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentHP = MaxHP;
        PS->MaxHP     = MaxHP;
        PS->CurrentMP = MaxMP;
        PS->MaxMP     = MaxMP;

        UE_LOG(LogTemp, Warning, TEXT("MaxHP: %f"), PS->MaxHP);
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

void APTPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APTPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APTPlayerCharacter::OnDeath()
{
    Super::OnDeath();

    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(HPRegenTimerHandle);

    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }

    OnPlayerDied.Broadcast();

    // 멀티플레이어 환경에서의 사망 후 리스폰 처리 시스템 연동
    class APlayerController* PC = Cast<APlayerController>(GetController());
    class APTGameMode* GM = Cast<APTGameMode>(GetWorld()->GetAuthGameMode());

    if (GM && PC)
    {
        // 죽은 캐릭터와 분리되기 전, 기억해둔 데이터(리스폰 위치)를 백업한다
        FVector SavedLoc = FVector::ZeroVector;
        bool bHasLoc = false;

        class APTBasePlayerState* PS = PC->GetPlayerState<class APTBasePlayerState>();
        if (PS && PS->HasRespawnLocation())
        {
            SavedLoc = PS->GetSavedRespawnLocation();
            bHasLoc = true;
        }

        // 백업한 데이터를 게임모드 리스폰 함수 인자에 넣는다
        GM->RespawnPlayer(PC, SavedLoc, bHasLoc);

        // 이제 안심하고 죽은 캐릭터와 분리해도 데이터가 유실되지 않는다
        PC->UnPossess();
    }

    // 분리되서 껍데기만 남은 캐릭터는 메모리에서 소멸시킨다
    Destroy();
}

void APTPlayerCharacter::TryInteract()
{
    // 공격 중일 때는 차단
    if (bIsAttacking) return;

    // PlayerState의 체력 장부를 검사하여 사망 시 차단 처리
    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (PS && PS->CurrentHP <= 0) return;

    // 1. 레이저(Line Trace) 시작점과 끝점 계산
    FVector StartLoc   = GetActorLocation();
    FVector ForwardVec = GetActorForwardVector();
    FVector EndLoc     = StartLoc + (ForwardVec * 200.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // 본인 제외

    // 2. ECC_Visibility 채널 충돌 스캔
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, QueryParams);

    // 디버그용 붉은 레이저 그리기
    DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Red, false, 2.0f, 0, 2.0f);

    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();
        UE_LOG(LogTemp, Log, TEXT("[상호작용 감지] 조준 대상: %s"), *HitActor->GetName());

        if (!HasAuthority())
        {
            Server_TryInteract(HitActor);
        }
        else
        {
            Server_TryInteract_Implementation(HitActor);
        }
    }
}

// 상호작용 Server RPC 구현부
void APTPlayerCharacter::Server_TryInteract_Implementation(AActor* TargetActor)
{
    if (!TargetActor) return;

    // 거리 2차 검증 (핵 방지용 보안 장부 체크)
    float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    if (Distance > 350.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[서버 보안 경고] 유저가 너무 먼 곳과 상호작용을 시도함. (거리: %f)"), Distance);
        return;
    }

    // 인터페이스 장착 여부 확인 및 최종 실행 명령
    if (TargetActor->GetClass()->ImplementsInterface(UPTInteractableInterface::StaticClass()))
    {
        UE_LOG(LogTemp, Log, TEXT("[서버 최종 승인] 인터페이스 실행 성공"));
        IPTInteractableInterface::Execute_Interact(TargetActor, this);
    }
}

bool APTPlayerCharacter::Server_TryInteract_Validate(AActor* TargetActor)
{
    if (!TargetActor) return false;
    return true;
}

void APTPlayerCharacter::Server_UseSkill_Implementation(FName SkillID)
{
    if (!HasAuthority()) return;

    if (SkillComp)
    {
        SkillComp->TryActivateSkill(SkillID);
    }
}

void APTPlayerCharacter::Server_PlayAttackMontage_Implementation(int32 MontageIndex)
{
    Multicast_PlayAttackMontage(MontageIndex);
}

void APTPlayerCharacter::Multicast_PlayAttackMontage_Implementation(int32 MontageIndex)
{
    if (IsLocallyControlled()) return;

    if (AttackMontages.IsValidIndex(MontageIndex))
    {
        PlayAnimMontage(AttackMontages[MontageIndex]);
    }
}

void APTPlayerCharacter::Server_Dodge_Implementation()
{
    Multicast_PlayDodgeMontage();
}

void APTPlayerCharacter::Multicast_PlayDodgeMontage_Implementation()
{
    if (IsLocallyControlled()) return;
    PlayAnimMontage(DodgeMontage);
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

void APTPlayerCharacter::OnDeath()
{
    Super::OnDeath();

    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(HPRegenTimerHandle);
    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }
    OnPlayerDied.Broadcast();

    // 멀티플레이어 환경에서의 사망 후 리스폰 처리 시스템 연동
    class APlayerController* PC = Cast<APlayerController>(GetController());
    class APTGameMode* GM = Cast<APTGameMode>(GetWorld()->GetAuthGameMode());

    if (GM && PC)
    {
        PC->UnPossess();
        GM->RespawnPlayer(PC);
    }

    // 분리되서 껍데기만 남은 캐릭터는 메모리에서 소멸시킨다
    Destroy();
}

void APTPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
