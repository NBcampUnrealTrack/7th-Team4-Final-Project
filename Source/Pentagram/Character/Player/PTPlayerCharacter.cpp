#include "Character/Player/PTPlayerCharacter.h"

#include "PTPlayerController.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTCharacterRow.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Core/PTGameMode.h"
#include "Core/Subsystems/PTSaveSubsystem.h"
#include "Animation/AnimInstance.h"
#include "Engine/GameInstance.h"
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

    ArmorChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmorChestMesh"));
    ArmorChestMesh->SetupAttachment(GetMesh());
    ArmorChestMesh->SetAnimationMode(EAnimationMode::AnimationCustomMode);
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

    ArmorChestMesh->SetLeaderPoseComponent(GetMesh());

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

    ApplyWeaponAnimLayer(CurrentWeaponType);

    PrewarmWeaponAnimLayers();
}

void APTPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (HasAuthority())
    {
        APTBasePlayerState* PTPlayerState = GetPlayerState<APTBasePlayerState>();
        UGameInstance* GameInstance = GetGameInstance();
        UPTSaveSubsystem* SaveSubsystem =
            GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
        if (PTPlayerState != nullptr && SaveSubsystem != nullptr)
        {
            SaveSubsystem->SavePlayer(PTPlayerState);
        }
    }

    Super::EndPlay(EndPlayReason);
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
    DOREPLIFETIME(APTPlayerCharacter, bIsInCombat);
    DOREPLIFETIME(APTPlayerCharacter, CurrentWeaponType);
}

void APTPlayerCharacter::OnDeath()
{
    Super::OnDeath();

    if (!HasAuthority()) return;

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

    if (APTPlayerController* PC = Cast<APTPlayerController>(GetController()))
    {
        PC->Client_ShowDeathMenu();
    }
}

void APTPlayerCharacter::TryInteract()
{
    // 공격 중일 때는 차단
    if (SkillComp->bIsAttacking) return;

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

void APTPlayerCharacter::AddInvincibility()
{
    if (!HasAuthority()) return;

    ++InvincibleRefs;

    bIsInvincible = (InvincibleRefs > 0);
}

void APTPlayerCharacter::RemoveInvincibility()
{
    if (!HasAuthority()) return;

    InvincibleRefs = FMath::Max(0, InvincibleRefs - 1);

    bIsInvincible = (InvincibleRefs > 0);
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

void APTPlayerCharacter::Server_UseSkill_Implementation(FName SkillID, FVector_NetQuantize InTargetLoc, FVector_NetQuantizeNormal InAimDir, AActor* InTargetActor)
{
    if (!HasAuthority() || !SkillComp) return;

    FPTSkillActivationRequest Request;
    Request.SkillRowName   = SkillID;
    Request.SkillDataTable = SkillComp->SkillDataTable;

    if (const FPTSkillRow* Row = SkillComp->GetSkillData(SkillID))
    {
        const FVector Origin = GetActorLocation();

        FVector To = FVector(InTargetLoc) - Origin; To.Z = 0.f;
        const float Clamped = FMath::Min(To.Size(), Row->CastRange);
        const FVector Dir = To.IsNearlyZero() ? GetActorForwardVector() : To.GetSafeNormal();

        Request.TargetLocation = Origin + Dir * Clamped;
        Request.AimDirection   = InAimDir.IsNearlyZero() ? GetActorForwardVector() : FVector(InAimDir);

        if (Row->TargetingMode == ESkillTargetingMode::Targeted && InTargetActor)
        {
            if (FVector::Dist2D(Origin, InTargetActor->GetActorLocation()) <= Row->CastRange)
                Request.TargetActor = InTargetActor;   // 사거리 내에서만 유효
        }
    }

    SkillComp->TryActivateSkill(Request);
}

void APTPlayerCharacter::OnDodgeInvincibleStart()
{
    if (!IsLocallyControlled()) return;

    if (SkillComp) SkillComp->Server_SetInvincible(true);
}

void APTPlayerCharacter::OnDodgeInvincibleEnd()
{
    if (!IsLocallyControlled()) return;

    if (SkillComp) SkillComp->Server_SetInvincible(false);
}

void APTPlayerCharacter::OnChannelSkillActivateNotify()
{
    if (!IsLocallyControlled()) return;

    if (SkillComp) SkillComp->Server_ChannelActivate();
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

void APTPlayerCharacter::RespawnAtLocation(const FVector& RespawnLocation)
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentHP = MaxHP;

    if (APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>())
    {
        PS->CurrentHP = CurrentHP;
        PS->MaxHP = MaxHP;
        PS->BroadcastAllStats();
    }

    SetActorLocation(RespawnLocation, false, nullptr, ETeleportType::TeleportPhysics);

    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->StopMovementImmediately();
        MovementComponent->SetMovementMode(MOVE_Walking);
    }

    if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    Multicast_ResetAfterRespawn();
}

void APTPlayerCharacter::Multicast_ResetAfterRespawn_Implementation()
{
    if (USkeletalMeshComponent* MeshComponent = GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
        {
            AnimInstance->StopAllMontages(0.15f);
        }
    }

    SkillComp->bIsAttacking = false;
    SkillComp->bCanCombo = false;
    SkillComp->ComboIndex = 0;
    bIsUsingSkill = false;
    bIsDodging = false;

    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->StopMovementImmediately();
        MovementComponent->SetMovementMode(MOVE_Walking);
    }

    if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

/*
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
*/

// 무기 외형 실시간 변경
void APTPlayerCharacter::UpdateWeaponVisual(const TSoftObjectPtr<UStaticMesh>& NewMeshAsset, const FItemData& ItemData)
{
    UE_LOG(LogTemp, Warning, TEXT("[UpdateWeaponVisual] 호출됨, IsNull: %d"), NewMeshAsset.IsNull());

    if (!WeaponMeshComp) return;

    EWeaponType NewWeaponType = EWeaponType::Hands;

    if (NewMeshAsset.IsNull())
    {
        // 빈 에셋이 오면 무기를 장착 해제한 것이므로 메시를 비웁니다.
        WeaponMeshComp->SetStaticMesh(nullptr);
        UE_LOG(LogTemp, Log, TEXT("[비주얼] 무기 외형 제거 완료"));
        NewWeaponType = EWeaponType::Hands;
    }
    else
    {
        UStaticMesh* LoadedMesh = NewMeshAsset.LoadSynchronous();
        if (LoadedMesh)
        {
            WeaponMeshComp->SetStaticMesh(LoadedMesh);
            UE_LOG(LogTemp, Log, TEXT("무기 외형 변경 완료: %s"), *LoadedMesh->GetName());

            CurrentWeaponItemData = ItemData;
        }
        NewWeaponType = ItemData.WeaponType;
    }

    CurrentWeaponType = NewWeaponType;

    if (NewWeaponType != EWeaponType::Hands)
    {
        AttachWeaponToSocket(bIsInCombat);
    }

    ApplyWeaponAnimLayer(NewWeaponType);
}

void APTPlayerCharacter::UpdateArmorVisual(EEquipSlotType SlotType, TSoftObjectPtr<USkeletalMesh> ArmorMesh)
{
    USkeletalMeshComponent* TargetComp = nullptr;
    switch (SlotType)
    {
        case EEquipSlotType::Chest:  TargetComp = ArmorChestMesh;  break;
        //case EEquipSlotType::Helmet: TargetComp = ArmorHelmetMesh; break;
        //case EEquipSlotType::Gloves: TargetComp = ArmorGlovesMesh; break;
        //case EEquipSlotType::Boots:  TargetComp = ArmorBootsMesh; break;
        default: return;
    }
    if (!TargetComp) return;

    if (ArmorMesh.IsNull())
    {
        TargetComp->SetSkeletalMesh(nullptr); // 벗기기
        return;
    }

    USkeletalMesh* Loaded = ArmorMesh.LoadSynchronous();
    TargetComp->SetSkeletalMesh(Loaded);
    TargetComp->SetLeaderPoseComponent(GetMesh());
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

void APTPlayerCharacter::ApplyWeaponAnimLayer(EWeaponType NewWeaponType)
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
    if (!AnimInst) return;

    TSubclassOf<UAnimInstance> LayerClass = nullptr;
    switch (NewWeaponType)
    {
    case EWeaponType::Sword: LayerClass = SwordAnimLayerClass; break;
    case EWeaponType::Bow:   LayerClass = BowAnimLayerClass;   break;
    case EWeaponType::Wand:  LayerClass = WandAnimLayerClass;  break;
    case EWeaponType::Hands:
    default:                LayerClass = HandAnimLayerClass;  break;
    }

    // 이미 같은 레이어가 링크돼 있으면 아무것도 하지 않고 끝
    if (LayerClass == CurrentLinkedAnimLayerClass)
    {
        CurrentWeaponType = NewWeaponType;
        return;
    }

    //무기 종류가 바뀌면 이때만 언링크
    if (CurrentLinkedAnimLayerClass)
    {
        AnimInst->UnlinkAnimClassLayers(CurrentLinkedAnimLayerClass);
    }

    if (LayerClass)
    {
        MeshComp->LinkAnimClassLayers(LayerClass);
        UE_LOG(LogTemp, Warning, TEXT("무기 애님레이어 적용중 : %s"), *LayerClass->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("무기 애님레이어 타입 %d가 비었음"), (int32)NewWeaponType);
    }

    // 새로 링크한 클래스를 반드시 기록해둬야 다음번에 정상적으로 Unlink 가능
    CurrentLinkedAnimLayerClass = LayerClass;
    CurrentWeaponType = NewWeaponType;
}

void APTPlayerCharacter::EnterCombat()
{
    if (!bIsInCombat && !bIsTransitioningToCombat)
    {
        bIsTransitioningToCombat = true;
        GetWorldTimerManager().SetTimer(
            CombatTransitionTimerHandle,
            this,
            &APTPlayerCharacter::OnCombatTransitionFinished,
            NormalToCombatTransitionDuration,
            false);
    }

    if (HasAuthority())
    {
        bIsInCombat = true;
        StartCombatExitTimer();
    }
    else
    {
        Server_EnterCombat();
    }
}

void APTPlayerCharacter::StartCombatExitTimer()
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(CombatExitTimerHandle);
    GetWorldTimerManager().SetTimer(
        CombatExitTimerHandle,
        this,
        &APTPlayerCharacter::OnCombatExitTimerExpired,
        8.f,
        false
        );
}

void APTPlayerCharacter::OnCombatExitTimerExpired()
{
    if (!HasAuthority()) return;

    bIsInCombat = false;
}

void APTPlayerCharacter::PrewarmWeaponAnimLayers()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
    if (!AnimInst) return;

    TArray<TSubclassOf<UAnimInstance>> AllLayers = { SwordAnimLayerClass, BowAnimLayerClass, WandAnimLayerClass };

    for (auto& Layer : AllLayers)
    {
        if (Layer)
        {
            MeshComp->LinkAnimClassLayers(Layer);
            MeshComp->UnlinkAnimClassLayers(Layer);
        }
    }

    // 원래 무기 레이어로 복구
    ApplyWeaponAnimLayer(CurrentWeaponType);
}

void APTPlayerCharacter::AttachWeaponToSocket(bool bToHand)
{
    if (!WeaponMeshComp || CurrentWeaponType == EWeaponType::Hands) return;

    FName Socket = bToHand
        ? GetHandSocket(CurrentWeaponType)
        : GetHolsterSocket(CurrentWeaponType);

    WeaponMeshComp->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::KeepRelativeTransform,
            Socket);

    if (bToHand)
    {
        WeaponMeshComp->SetRelativeLocation(CurrentWeaponItemData.WeaponRelativeLocation);
        WeaponMeshComp->SetRelativeRotation(CurrentWeaponItemData.WeaponRelativeRotation);
        WeaponMeshComp->SetRelativeScale3D(CurrentWeaponItemData.WeaponRelativeScale);
    }
    else
    {
        WeaponMeshComp->SetRelativeLocation(CurrentWeaponItemData.WeaponHolsterRelativeLocation);
        WeaponMeshComp->SetRelativeRotation(CurrentWeaponItemData.WeaponHolsterRelativeRotation);
        WeaponMeshComp->SetRelativeScale3D(CurrentWeaponItemData.WeaponHolsterRelativeScale);
    }
}

void APTPlayerCharacter::OnCombatTransitionFinished()
{
    bIsTransitioningToCombat = false;

    if (SkillComp)
    {
        SkillComp->PlayPendingAction();
    }
}

void APTPlayerCharacter::EquipArmorChest(USkeletalMesh* NewArmorMesh)
{
    ArmorChestMesh->SetSkeletalMesh(NewArmorMesh);
    ArmorChestMesh->SetLeaderPoseComponent(GetMesh());
}

void APTPlayerCharacter::OnRep_CurrentWeaponType()
{
    ApplyWeaponAnimLayer(CurrentWeaponType);
}

void APTPlayerCharacter::Server_EnterCombat()
{
    bIsInCombat = true;
    StartCombatExitTimer();
}

FName APTPlayerCharacter::GetHolsterSocket(EWeaponType Type) const
{
    switch (Type)
    {
        case EWeaponType::Sword: return TEXT("sword_holster");
        case EWeaponType::Wand:  return TEXT("wand_back");
        case EWeaponType::Bow:   return TEXT("Bow_back");
        default:                 return NAME_None;
    }
}

FName APTPlayerCharacter::GetHandSocket(EWeaponType Type) const
{
    switch (Type)
    {
        case EWeaponType::Sword: return TEXT("weapon_r");
        case EWeaponType::Wand:  return TEXT("weapon_wand");
        case EWeaponType::Bow:   return TEXT("weapon_bow");
        default:                 return TEXT("weapon_r");
    }
}
