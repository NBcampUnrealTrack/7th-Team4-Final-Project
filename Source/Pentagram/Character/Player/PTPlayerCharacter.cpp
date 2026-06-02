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
#include "PTInventoryComponent.h" 
#include "PTEquipmentComponent.h" 

// 충돌 및 디버그 라인을 그리기 위함 
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#include "Core/Interface/PTInteractableInterface.h" 

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

    InventoryComponent = CreateDefaultSubobject<UPTInventoryComponent>(TEXT("InventoryComponent"));
    EquipmentComponent = CreateDefaultSubobject<UPTEquipmentComponent>(TEXT("EquipmentComponent"));

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

// 클라이언트/서버 공용 상호작용 시도 함수
// 클라이언트가 Line Trace를 쏴서 상호작용 대상을 감지 
void APTPlayerCharacter::TryInteract()
{
    // 공격 중이거나 죽었을 때는 상호작용 차단 
    if (bIsAttacking || CurrentHP <= 0) return; 

    // 1. 레이저(Line Trace) 시작점과 끝점 계산 (캐릭터 위치에서 정면으로 200cm = 2미터)
    FVector StartLoc = GetActorLocation();
    FVector ForwardVec = GetActorForwardVector();
    FVector EndLoc = StartLoc + (ForwardVec * 200.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // 본인 제외

    // 2. 레이저 발사 (ECC_Visibility 채널을 이용해 충돌 스캔)
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, QueryParams);

    // 디버그용 붉은 레이저 그리기 (테스트용 붉은 선 2초간 유지, 완료 후 주석 처리 가능) 
    DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Red, false, 2.0f, 0, 2.0f);

    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();
        UE_LOG(LogTemp, Log, TEXT("[상호작용 감지] 조준 대상: %s"), *HitActor->GetName());

        // [멀티플레이어 핵심 분기] 
        // 내 컴퓨터가 클라이언트라면 서버에게 "이 액터랑 상호작용 하겠다" 무전(RPC)을 쏩니다.
        if (!HasAuthority())
        {
            Server_TryInteract(HitActor);
        }
        else
        {
            // 내가 이미 서버라면 직접 RPC 구현부를 즉시 실행. 
            Server_TryInteract_Implementation(HitActor);
        }
    }
}

// 상호작용 Server RPC 구현부 (오직 서버 컴퓨터에서만 실행됨)
void APTPlayerCharacter::Server_TryInteract_Implementation(AActor* TargetActor)
{
    if (!TargetActor) return;

    // 거리 2차 검증 (클라이언트가 핵을 써서 멀리서 F키를 누른 건지 장부 체크)
    float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    if (Distance > 250.0f) // 오차 범위를 감안해 조금 넉넉하게 2.5미터 제한
    {
        UE_LOG(LogTemp, Warning, TEXT("[서버 보안 경고] 유저가 너무 먼 곳의 오브젝트와 상호작용을 시도함. (거리: %f)"), Distance);
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
    // 타겟 액터가 존재하지 않는 찌꺼기 패킷이면 차단
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

}
