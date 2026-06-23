#include "Character/Player/PTPlayerCharacter.h"

#include "PTPlayerController.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTCharacterRow.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
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
#include "Interface/PTInteractableInterface.h"

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
    SpringArmComp->bInheritRoll  = false;
    SpringArmComp->bInheritYaw   = false;
    SpringArmComp->bEnableCameraLag  = false;
    SpringArmComp->bDoCollisionTest  = false;
    SpringArmComp->SocketOffset = FVector(0.f, 0.f, 0.f);

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
    CameraComp->bUsePawnControlRotation = false;

    SkillComp          = CreateDefaultSubobject<UPTPlayerSkillComponent>(TEXT("Skill"));
    InventoryComponent = CreateDefaultSubobject<UPTInventoryComponent>(TEXT("InventoryComponent"));
    EquipmentComponent = CreateDefaultSubobject<UPTEquipmentComponent>(TEXT("EquipmentComponent"));

    WeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComp"));
    if (GetMesh())
    {
        // 무기 소켓에 붙일 메시 컴포넌트 설정
        WeaponMeshComp->SetupAttachment(GetMesh(), TEXT("weapon_right"));
    }
    // 공격 판정은AnimNotify에서 처리하므로 무기 자체의 물리 충돌은 꺼둠
    WeaponMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GetCharacterMovement()->bOrientRotationToMovement    = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
    bUseControllerRotationYaw = false;

    CharacterType = ECharacterType::Player;
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

        GetWorldTimerManager().SetTimer(
            MPRegenTimerHandle,
            this,
            &APTPlayerCharacter::RegenMP,
            3.f,
            true
            );
    }
}

void APTPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 개인의 플레이어 화면에서만 레이트레이스를 발사하도록 최적화
    if (!IsLocallyControlled() || !CameraComp) return;

    // 시작점(카메라 위치), 끝점(캐릭터 위치) 설정
    FVector StartPos = CameraComp->GetComponentLocation();
    FVector EndPos = GetActorLocation();

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // 나 자신은 검사 대상에서 필터링

    // 카메라와 내 몸 사이에 Visibility 채널 기준의 장애물이 있는지 실시간 체크
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECC_WorldDynamic, QueryParams);

    if (bHit && HitResult.GetActor())
    {
        AActor* CurrentHitActor = HitResult.GetActor();

        // 새로 충돌한 물체가 기존 충돌 물체와 다른 새로운 장애물일 때만 신호 발송
        if (CurrentHitActor != LastHidingActor)
        {
            if (LastHidingActor)
            {
                OnStructureUnHidden(LastHidingActor);
            }

            // 블루프린트로 전송: 이 물체를 가려라!
            OnStructureHidden(CurrentHitActor);
            LastHidingActor = CurrentHitActor;
        }
    }
    else
    {
        // 장애물 영역을 완전히 벗어났을 경우 기존 장애물 원상복구
        if (LastHidingActor)
        {
            OnStructureUnHidden(LastHidingActor);
            LastHidingActor = nullptr;
        }
    }
}

void APTPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTPlayerCharacter, AtkBuffBonus);
    DOREPLIFETIME(APTPlayerCharacter, bIsUsingSkill);
}

void APTPlayerCharacter::OnDeath()
{
    Super::OnDeath();

    if (!HasAuthority()) return

    GetWorldTimerManager().ClearTimer(HPRegenTimerHandle);

    Multicast_PlayDeathMontage();

    OnPlayerDied.Broadcast();

    // // 멀티플레이어 환경에서의 사망 후 리스폰 처리 시스템 연동
    // class APlayerController* PC = Cast<APlayerController>(GetController());
    // class APTGameMode* GM       = Cast<APTGameMode>(GetWorld()->GetAuthGameMode());
    //
    // if (GM && PC)
    // {
    //     // 죽은 캐릭터와 분리되기 전, 기억해둔 데이터(리스폰 위치)를 백업한다
    //     FVector SavedLoc = FVector::ZeroVector;
    //     bool bHasLoc     = false;
    //
    //     class APTBasePlayerState* PS = PC->GetPlayerState<class APTBasePlayerState>();
    //     if (PS && PS->HasRespawnLocation())
    //     {
    //         SavedLoc = PS->GetSavedRespawnLocation();
    //         bHasLoc  = true;
    //     }
    //
    //     // 백업한 데이터를 게임모드 리스폰 함수 인자에 넣는다
    //     GM->RespawnPlayer(PC, SavedLoc, bHasLoc);
    //
    //     // 이제 안심하고 죽은 캐릭터와 분리해도 데이터가 유실되지 않는다
    //     PC->UnPossess();
    // }
    //
    // // 분리되서 껍데기만 남은 캐릭터는 메모리에서 소멸시킨다
    // Destroy();

    if (APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>())
    {
        PS->SetSavedRespawnLocation(GetActorLocation());
    }

    if (APTPlayerController* PC = Cast<APTPlayerController>(GetController()))
    {
        PC->Client_ShowDeathMenu();
    }
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
        FPTSkillActivationRequest Request;
        Request.SkillRowName    = SkillID;
        Request.SkillDataTable  = SkillComp->SkillDataTable;

        SkillComp->TryActivateSkill(Request);
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

void APTPlayerCharacter::OnDodgeInvincibleStart()
{
    // 로컬 클라이언트에서 AnimNotify 발동 → 서버로 무적 ON 전달
    if (SkillComp)
    {
        SkillComp->Server_SetInvincible(true);
    }
}

void APTPlayerCharacter::OnDodgeInvincibleEnd()
{
    // 로컬 클라이언트에서 AnimNotify 발동 → 서버로 무적 OFF 전달
    if (SkillComp)
    {
        SkillComp->Server_SetInvincible(false);
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

void APTPlayerCharacter::RegenMP()
{
    if (!HasAuthority()) return;
    float RegenAmount = MaxMP * 0.1f;
    CurrentMP = FMath::Min(CurrentMP + RegenAmount, MaxMP);

    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentMP = CurrentMP;
    }
}

float APTPlayerCharacter::GetTotalAttack() const
{
    // 부모 클래스(PTBaseCharacter)가 데이터 테이블로부터 플레이어의 순수 기본 공격력을 가져옴.
    float FinalAttack = BaseAtk;

    // 장비창 컴포넌트가 정상적으로 장착되어 있다면, 현재 장착 중인 아이템들의 보너스 STR 수치를 합산
    if (IsValid(EquipmentComponent))
    {
        // 장비 스탯 총합 Getter 함수를 활용
        FinalAttack += static_cast<float>(EquipmentComponent->GetTotalBonusStr());
    }

    //공격 버프 효과 합산
    FinalAttack *= (1.f + AtkBuffBonus);

    return FinalAttack;
}

void APTPlayerCharacter::Multicast_PlayDeathMontage_Implementation()
{
    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }
}

void APTPlayerCharacter::Server_StopAttack_Implementation()
{
    Multicast_StopAttack();
}

void APTPlayerCharacter::Multicast_StopAttack_Implementation()
{
    StopAnimMontage();
    bIsAttacking = false;
    bCanCombo    = false;
    ComboIndex   = 0;
}

// 무기 외형 실시간 변경
void APTPlayerCharacter::UpdateWeaponVisual(const TSoftObjectPtr<UStaticMesh>& NewMeshAsset)
{
    UE_LOG(LogTemp, Warning, TEXT("[UpdateWeaponVisual] 호출됨, IsNull: %d"), NewMeshAsset.IsNull());

    if (!WeaponMeshComp) return;

    if (NewMeshAsset.IsNull())
    {
        // 빈 에셋이 오면 무기를 장착 해제한 것이므로 메시를 비웁니다.
        WeaponMeshComp->SetStaticMesh(nullptr);
        UE_LOG(LogTemp, Log, TEXT("[비주얼] 무기 외형 제거 완료"));
    }
    else
    {
        // SoftObjectPtr이므로 안전하게 동기식 로드(LoadSynchronous)하여 메시를 채웁니다.
        UStaticMesh* LoadedMesh = NewMeshAsset.LoadSynchronous();
        if (LoadedMesh)
        {
            WeaponMeshComp->SetStaticMesh(LoadedMesh);
            UE_LOG(LogTemp, Log, TEXT("[비주얼] 무기 외형 변경 완료: %s"), *LoadedMesh->GetName());
        }
    }
}

void APTPlayerCharacter::ApplyBuff(float BonusMultiplier, float Duration)
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(BuffTimerHandle);

    AtkBuffBonus = BonusMultiplier;

    GetWorld()->GetTimerManager().SetTimer(
        BuffTimerHandle,
        this,
        &APTPlayerCharacter::OnAtkBuffExpired,
        Duration,
        false
        );
}

void APTPlayerCharacter::OnAtkBuffExpired()
{
    AtkBuffBonus = 0.f;
}
