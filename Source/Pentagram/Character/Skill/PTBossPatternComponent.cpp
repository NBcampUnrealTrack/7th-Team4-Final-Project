#include "Character/Skill/PTBossPatternComponent.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Character/Monsters/Projectile/PTBossProjectile.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "Character/Skill/PTMonsterSkillComponent.h"
#include "Character/Skill/PTSkillComponent.h"
#include "Character/PTBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h" 
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Character/Monsters/Skill/PTAreaWarning.h"
#include "GameFramework/CharacterMovementComponent.h"


UPTBossPatternComponent::UPTBossPatternComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UPTBossPatternComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPTBossPatternComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(ActiveLaserAudioComp))
    {
        ActiveLaserAudioComp->Stop();
        ActiveLaserAudioComp = nullptr;
    }

    StopSafeZoneFXLocal();

    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (auto& Pair : PatternCooldownTimers)
        {
            World->GetTimerManager().ClearTimer(Pair.Value);
        }

        for (FTimerHandle& Handle : AreaAttackTimers)
        {
            World->GetTimerManager().ClearTimer(Handle);
        }

        AreaAttackTimers.Empty();
    }

    bAreaAttackInProgress = false;

    ClearProjectileTimers();

    Super::EndPlay(EndPlayReason);
}

void UPTBossPatternComponent::PreloadAllSkills()
{
    if (!IsValid(BossSkillDataTable))
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] BossSkillDataTable 미설정"));
#endif
        bSkillAssetsLoaded = false;
        return;
    }

    const int32 TotalRows = Phase0SkillRowNames.Num() + Phase1SkillRowNames.Num() + Phase2SkillRowNames.Num();
    if (TotalRows <= 0)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] Preload 실패 — Phase RowName 목록이 비어 있음"));
#endif
        bSkillAssetsLoaded = false;
        return;
    }

    int32 FailCount = 0;

    auto LoadRow = [&FailCount](FPTBossSkillRow* Row, const FName& RowName)
        {
            if (!Row)
            {
#if !UE_BUILD_SHIPPING
                UE_LOG(LogTemp, Warning, TEXT("[BossPattern] Preload 실패 — [%s] RowName 또는 RowStruct 확인"), *RowName.ToString());
#endif
                ++FailCount;
                return;
            }

            Row->SkillMontage.LoadSynchronous();
            Row->OverrideMontage.LoadSynchronous();
            Row->SkillEffect.LoadSynchronous();
            Row->SkillSound.LoadSynchronous();
            Row->SkillHitSound.LoadSynchronous();
            Row->AreaFallEffect.LoadSynchronous();
            Row->AreaCastEffect.LoadSynchronous();
            Row->AreaImpactEffect.LoadSynchronous();
            Row->SafeZoneEffect.LoadSynchronous();
            Row->LaserEffect.LoadSynchronous();
            Row->LaserLoopSound.LoadSynchronous();
        };

    for (const FName& RowName : Phase0SkillRowNames)
    {
        LoadRow(BossSkillDataTable->FindRow<FPTBossSkillRow>(RowName, TEXT("")), RowName);
    }

    for (const FName& RowName : Phase1SkillRowNames)
    {
        LoadRow(BossSkillDataTable->FindRow<FPTBossSkillRow>(RowName, TEXT("")), RowName);
    }

    for (const FName& RowName : Phase2SkillRowNames)
    {
        LoadRow(BossSkillDataTable->FindRow<FPTBossSkillRow>(RowName, TEXT("")), RowName);
    }

    if (FailCount > 0)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] %d / %d Row 로드 실패 — 일부 스킬이 동작하지 않을 수 있음"),
            FailCount, TotalRows);
#endif
    }

    bSkillAssetsLoaded = (FailCount < TotalRows);
}

float UPTBossPatternComponent::ExecuteSkillForPhase(int32 Phase)
{
    if (!bSkillAssetsLoaded)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] 스킬 에셋 미로드 — PreloadAllSkills 호출 확인"));
#endif
        return 0.f;
    }

    if (!IsValid(SkillComponent))
    {
        return 0.f;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return 0.f;
    }

    if (bIsLaserActive)
    {
        return 0.f;
    }

    if (bHasPendingSkill)
    {
        return 0.f;
    }

    bHasPendingSkill = false;

    auto [RowName, Row] = PickNextSkill(Phase);
    if (!Row)
    {
        return 0.f;
    }

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Log, TEXT("[BossPattern] 선택 성공 — %s"), *RowName.ToString());
#endif

    FPTSkillActivationRequest Request;
    Request.SkillRowName = RowName;
    Request.SkillDataTable = BossSkillDataTable;
    Request.OverrideMontage = Row->OverrideMontage;

    const bool bActivated = SkillComponent->TryActivateSkillChecked(Request);
    if (!bActivated)
    {
        return 0.f;
    }

    PendingSkillSnapshot = *Row;
    bHasPendingSkill = true;
#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Log, TEXT("[BossPattern] Pending Skill Set: %s"), *RowName.ToString());
#endif

    if (Row->PatternCooldown > 0.f)
    {
        UWorld* World = GetWorld();
        if (IsValid(World))
        {
            PatternCooldownFlags.FindOrAdd(RowName) = true;
            FTimerHandle& Timer = PatternCooldownTimers.FindOrAdd(RowName);

#if !UE_BUILD_SHIPPING
            UE_LOG(LogTemp, Log, TEXT("[BossPattern] PatternCooldown 시작 — %s (%.1f초)"),
                *RowName.ToString(), Row->PatternCooldown);
#endif
            World->GetTimerManager().ClearTimer(Timer);
            World->GetTimerManager().SetTimer(Timer,
                FTimerDelegate::CreateWeakLambda(this, [this, RowName]()
                {
                    PatternCooldownFlags.FindOrAdd(RowName) = false;
#if !UE_BUILD_SHIPPING
                    UE_LOG(LogTemp, Log, TEXT("[BossPattern] PatternCooldown 종료 — %s"), *RowName.ToString());
#endif
                }),
                Row->PatternCooldown, false);
        }
    }

    UAnimMontage* PlayedMontage = nullptr;

    if (!Row->OverrideMontage.IsNull())
    {
        PlayedMontage = Row->OverrideMontage.LoadSynchronous();
    }
    else if (!Row->SkillMontage.IsNull())
    {
        PlayedMontage = Row->SkillMontage.LoadSynchronous();
    }

    if (Row->SkillType == EBossSkillType::Laser)
    {
        if (APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MoveComp = Boss->GetCharacterMovement())
            {
                if (MoveComp->MaxWalkSpeed > 0.f)
                {
                    SavedMaxWalkSpeed = MoveComp->MaxWalkSpeed;
                }
                MoveComp->MaxWalkSpeed = 0.f;
                MoveComp->bOrientRotationToMovement = false;
            }
            if (AAIController* AIC = Cast<AAIController>(Boss->GetController()))
            {
                AIC->StopMovement();
            }
        }
        return Row->LaserDuration;
    }

    if (Row->SkillType == EBossSkillType::Area && Row->bHoldMontageUntilDelay)
    {
        return IsValid(PlayedMontage)
            ? PlayedMontage->GetPlayLength() + Row->AreaAttackDelay
            : Row->AreaAttackDelay;
    }

    return IsValid(PlayedMontage) ? PlayedMontage->GetPlayLength() : 0.f;
}

void UPTBossPatternComponent::ExecutePendingSkill()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    if (!bHasPendingSkill)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] ExecutePendingSkill 호출됐지만 대기 중인 스킬 없음 — 몽타주 노티파이 확인"));
#endif
        return;
    }

    bHasPendingSkill = false;

    switch (PendingSkillSnapshot.SkillType)
    {
    case EBossSkillType::Projectile:
        SpawnProjectile(PendingSkillSnapshot);
        break;

    case EBossSkillType::Area:
        SpawnAreaAttack(PendingSkillSnapshot);
        break;

    case EBossSkillType::Laser:
        SpawnLaser(PendingSkillSnapshot);
        break;

    case EBossSkillType::Melee:
        SpawnMeleeAttack(PendingSkillSnapshot);
        break;

    default:
        break;
    }
}

void UPTBossPatternComponent::SetSkillComponent(UPTMonsterSkillComponent* InSkillComponent)
{
    SkillComponent = InSkillComponent;
}

void UPTBossPatternComponent::ClearProjectileTimers()
{
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    for (FTimerHandle& Handle : ProjectileTimers)
    {
        World->GetTimerManager().ClearTimer(Handle);
    }

    ProjectileTimers.Empty();
}

void UPTBossPatternComponent::StopLaser()
{
    if (!bIsLaserActive)
    {
        StopLaserFXLocal();
        return;
    }

    bIsLaserActive = false;
    ClearLaserTimers();

    if (IsValid(ActiveLaserAudioComp))
    {
        ActiveLaserAudioComp->Stop();
        ActiveLaserAudioComp = nullptr;
    }

    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner());
    if (!IsValid(Boss))
    {
        MulticastResumeMontage();
        return;
    }

    if (UCharacterMovementComponent* MoveComp = Boss->GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = SavedMaxWalkSpeed;
    }

    UAnimInstance* AnimInst = Boss->GetMesh() ? Boss->GetMesh()->GetAnimInstance() : nullptr;
    UAnimMontage* ActiveMontage = AnimInst ? AnimInst->GetCurrentActiveMontage() : nullptr;

    if (!IsValid(AnimInst) || !IsValid(ActiveMontage))
    {
        bLaserPhaseActive = false;
    }
    else
    {
        FOnMontageEnded EndDelegate;
        TWeakObjectPtr<APTBossMonsterCharacter> WeakBoss(Boss);
        EndDelegate.BindWeakLambda(this, [this, WeakBoss](UAnimMontage*, bool)
        {
            bLaserPhaseActive = false;

            if (!WeakBoss.IsValid()) return;

            if (UCharacterMovementComponent* MC = WeakBoss->GetCharacterMovement())
            {
                MC->bOrientRotationToMovement = true;
                MC->bUseControllerDesiredRotation = true;
            }
            if (AAIController* AIC = Cast<AAIController>(WeakBoss->GetController()))
            {
                if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
                {
                    if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor)))
                    {
                        AIC->SetFocus(Target, EAIFocusPriority::Gameplay);
                    }
                }
            }
        });
        AnimInst->Montage_SetEndDelegate(EndDelegate, ActiveMontage);
    }

    MulticastResumeMontage();

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        MulticastStopLaserFX();
    }
    else
    {
        StopLaserFXLocal();
    }
}

void UPTBossPatternComponent::MulticastSpawnAreaWarningBatch_Implementation(const TArray<FVector>& DropLocations, float BaseDelay, float Interval, UNiagaraSystem* FallEffect, UNiagaraSystem* ImpactEffect, float StartHeight, TSubclassOf<APTAreaWarning> WarningClass, float MaxRadius, bool bGroundMode)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("[MulticastSpawnAreaWarning] HasAuth: %d"), GetOwner()->HasAuthority());
#endif

    UWorld* World = GetWorld();
    if (!IsValid(World) || !WarningClass)
    {
        return;
    }

    for (int32 i = 0; i < DropLocations.Num(); ++i)
    {
        FActorSpawnParameters Params;
        Params.Owner = GetOwner();

        APTAreaWarning* Area = World->SpawnActor<APTAreaWarning>(WarningClass, DropLocations[i], FRotator::ZeroRotator, Params);
        if (IsValid(Area))
        {
#if !UE_BUILD_SHIPPING
            UE_LOG(LogTemp, Warning, TEXT("[Multicast] MaxRadius 전달: %.1f"), MaxRadius);
#endif
            Area->Launch(
                DropLocations[i],
                BaseDelay + Interval * i,
                FallEffect, ImpactEffect,
                StartHeight, MaxRadius,
                bGroundMode
            );
        }
    }
}

void UPTBossPatternComponent::MulticastSpawnAreaFX_Implementation(FVector CastLocation, bool bHasSafeZone, FVector SafeZoneCenter, UNiagaraSystem* CastFX, UNiagaraSystem* SafeZoneFX, float SafeZoneRadius, float AreaAttackDelay)
{
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    if (CastFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, CastFX, CastLocation, FRotator::ZeroRotator);
    }

    if (bHasSafeZone && SafeZoneFX)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[SafeZone] SafeZoneRadius: %.1f"), SafeZoneRadius);
#endif

        StopSafeZoneFXLocal();

        ActiveSafeZoneComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, SafeZoneFX, SafeZoneCenter, FRotator::ZeroRotator, FVector::OneVector, false);
        if (IsValid(ActiveSafeZoneComp))
        {
            FTimerHandle& SafeZoneTimer = AreaAttackTimers.AddDefaulted_GetRef();
            World->GetTimerManager().SetTimer(
                SafeZoneTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
                    {
                        StopSafeZoneFXLocal();
                    }),
                AreaAttackDelay, false);
        }
    }
}

void UPTBossPatternComponent::MulticastPlayLaunchSound_Implementation(FVector Location, USoundBase* Sound)
{
    if (IsValid(Sound))
    {
        UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location);
    }
}

void UPTBossPatternComponent::MulticastStopSafeZoneFX_Implementation()
{
    StopSafeZoneFXLocal();
}

void UPTBossPatternComponent::ClearLaserTimers()
{
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().ClearTimer(LaserTickTimerHandle);
        World->GetTimerManager().ClearTimer(LaserEndTimerHandle);
    }
}

void UPTBossPatternComponent::SpawnMeleeAttack(const FPTBossSkillRow& RowSnapshot)
{
    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner());
    if (!IsValid(Boss) || !Boss->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnMeleeAttack] Boss 무효 또는 권한 없음"));
        return;
    }

    const float FinalDamage = Boss->GetBaseAtk()
        * RowSnapshot.DamageMultiplier
        * Boss->GetDamageMultiplierForPhase(Boss->GetCurrentPhase());

    Boss->SetMeleeAttackData(FinalDamage, RowSnapshot.MakeHitInfo(Boss));
}

TPair<FName, FPTBossSkillRow*> UPTBossPatternComponent::PickNextSkill(int32 Phase)
{
    if (!IsValid(BossSkillDataTable))
    {
        return { NAME_None, nullptr };
    }

    if (bIsLaserActive)
    {
        return { NAME_None, nullptr };
    }

    TArray<FName> RowNames = Phase0SkillRowNames;
    if (Phase >= 1) RowNames.Append(Phase1SkillRowNames);
    if (Phase >= 2) RowNames.Append(Phase2SkillRowNames);

    if (RowNames.IsEmpty())
    {
        return { NAME_None, nullptr };
    }

    AActor* OwnerActor = GetOwner();
    AActor* TargetActor = GetTargetActor();

    if (!IsValid(OwnerActor) || !IsValid(TargetActor))
    {
        return { NAME_None, nullptr };
    }

    const float DistToTarget = FVector::Dist(OwnerActor->GetActorLocation(), TargetActor->GetActorLocation());

    TArray<TPair<FName, FPTBossSkillRow*>> Candidates;
    float TotalWeight = 0.f;

    for (const FName& RowName : RowNames)
    {
        FPTBossSkillRow* Row = BossSkillDataTable->FindRow<FPTBossSkillRow>(RowName, TEXT("PickNextSkill"));

        if (!Row)
        {
#if !UE_BUILD_SHIPPING
            UE_LOG(LogTemp, Warning, TEXT("[BossPattern] [%s] Row 조회 실패 — 후보에서 제외"), *RowName.ToString());
#endif
            continue;
        }

        if (Row->Weight <= 0.f)
        {
#if !UE_BUILD_SHIPPING
            UE_LOG(LogTemp, Warning, TEXT("[BossPattern] [%s] Weight <= 0 — 후보에서 제외"), *RowName.ToString());
#endif
            continue;
        }

        if (PatternCooldownFlags.FindRef(RowName))
        {
            continue;
        }

        if (bAreaAttackInProgress && PendingSkillSnapshot.bHoldMontageUntilDelay)
        {
            continue;
        }

        if (Row->MinUseDistance > 0.f && DistToTarget < Row->MinUseDistance)
        {
            continue;
        }
        if (Row->MaxUseDistance > 0.f && DistToTarget > Row->MaxUseDistance)
        {
            continue;
        }

        TotalWeight += Row->Weight;
        Candidates.Add({ RowName, Row });
    }

    if (TotalWeight <= 0.f)
    {
        return { NAME_None, nullptr };
    }

    float Rand = FMath::FRandRange(0.f, TotalWeight);
    float Acc = 0.f;
    for (auto& [RowName, Row] : Candidates)
    {
        Acc += Row->Weight;
        if (Rand <= Acc)
        {
            return { RowName, Row };
        }
    }

    if (Candidates.Num() > 0)
    {
        return Candidates.Last();
    }

    return { NAME_None, nullptr };
}

void UPTBossPatternComponent::SpawnProjectile(const FPTBossSkillRow& RowSnapshot)
{
    if (!IsValid(ProjectileClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnProjectile] ProjectileClass 미설정 — BP에서 확인"));
        return;
    }

    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner());
    if (!IsValid(Boss) || !Boss->HasAuthority())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    ClearProjectileTimers();

    const int32 Count = FMath::Max(1, RowSnapshot.ProjectileCount);
    const float Interval = FMath::Max(0.f, RowSnapshot.ProjectileInterval);

    const float FinalDamage = Boss->GetBaseAtk()
        * RowSnapshot.DamageMultiplier
        * Boss->GetDamageMultiplierForPhase(Boss->GetCurrentPhase())
        / static_cast<float>(Count);

    auto FireProjectile = [this, Boss, RowSnapshot, World, FinalDamage]()
        {
            if (!IsValid(Boss) || !IsValid(World))
            {
                return;
            }

            USkeletalMeshComponent* Mesh = Boss->GetMesh();
            if (!IsValid(Mesh))
            {
                return;
            }

            const bool bUseSocket = ProjectileSpawnSocket != NAME_None && Mesh->DoesSocketExist(ProjectileSpawnSocket);
            const FVector SpawnLocation = bUseSocket
                ? Mesh->GetSocketLocation(ProjectileSpawnSocket)
                : Boss->GetActorLocation()
                + Boss->GetActorForwardVector() * ProjectileSpawnForwardOffset
                + FVector(0.f, 0.f, ProjectileSpawnHeightOffset);
            const FRotator SpawnRotation = Boss->GetActorRotation();
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = Boss;
            SpawnParams.Instigator = Boss;

            TSubclassOf<APTBossProjectile> SpawnClass = RowSnapshot.OverrideProjectileClass
                ? RowSnapshot.OverrideProjectileClass
                : ProjectileClass;
            if (!IsValid(SpawnClass))
            {
                UE_LOG(LogTemp, Warning, TEXT("[SpawnProjectile] SpawnClass 무효 — OverrideProjectileClass/ProjectileClass 확인"));
                return;
            }

            APTBossProjectile* Projectile = World->SpawnActor<APTBossProjectile>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
            if (!IsValid(Projectile))
            {
                UE_LOG(LogTemp, Warning, TEXT("[SpawnProjectile] SpawnActor 실패 — SpawnClass: %s"), *SpawnClass->GetName());
                return;
            }

            Projectile->IgnoreActor(Boss);

            AActor* Target = GetTargetActor();
            const FVector Direction = IsValid(Target) ? (Target->GetActorLocation() - SpawnLocation).GetSafeNormal() : Boss->GetActorForwardVector();

            FPTHitInfo HitInfo = RowSnapshot.MakeHitInfo(Boss);

            Projectile->Launch(Direction, FinalDamage, RowSnapshot.ProjectileSpeed, HitInfo, RowSnapshot.HomingStrength, Target);

            if (USoundBase* LaunchSound = RowSnapshot.SkillSound.Get())
            {
                MulticastPlayLaunchSound(SpawnLocation, LaunchSound);
            }
        };

    FireProjectile();

    if (Count > 1)
    {
        ProjectileTimers.SetNum(Count - 1);

        for (int32 i = 1; i < Count; ++i)
        {
            const float Delay = Interval * i;

            World->GetTimerManager().SetTimer(
                ProjectileTimers[i - 1],
                FTimerDelegate::CreateWeakLambda(this, FireProjectile),
                Delay, false);
        }
    }
}

void UPTBossPatternComponent::SpawnAreaAttack(const FPTBossSkillRow& RowSnapshot)
{
    AActor* Target = GetTargetActor();
    if (!IsValid(Target))
    {
        return;
    }

    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner());
    if (!IsValid(Boss) || !Boss->HasAuthority())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    for (FTimerHandle& Handle : AreaAttackTimers)
    {
        World->GetTimerManager().ClearTimer(Handle);
    }
    AreaAttackTimers.Empty();

    if (RowSnapshot.bHoldMontageUntilDelay)
    {
        USkeletalMeshComponent* BossMesh = Boss->GetMesh();
        if (IsValid(BossMesh))
        {
            if (UAnimInstance* AnimInstance = BossMesh->GetAnimInstance())
            {
                AnimInstance->Montage_Pause(nullptr);
            }
        }
    }

    const FVector BossLocation = Boss->GetActorLocation();
    const FVector TargetLocation = Target->GetActorLocation();

    const FVector BaseDropLocation = RowSnapshot.bHasDirectionalSafeZone ? BossLocation : TargetLocation;

    const int32 Count = FMath::Max(1, RowSnapshot.AreaAttackCount);
    TArray<FVector> DropLocations;
    DropLocations.Reserve(Count);

    for (int32 i = 0; i < Count; ++i)
    {
        FVector DropPos = BaseDropLocation;

        if (RowSnapshot.AreaAttackSpreadRadius > 0.f)
        {
            const float Angle = FMath::FRandRange(0.f, 360.f);
            const float Dist = FMath::FRandRange(0.f, RowSnapshot.AreaAttackSpreadRadius);
            DropPos.X += FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist;
            DropPos.Y += FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist;
        }

        DropLocations.Add(DropPos);
    }

    FVector SafeZoneCenter = FVector::ZeroVector;

    if (RowSnapshot.bHasDirectionalSafeZone)
    {
        const FVector Cardinals[] = {
            FVector(1.f,  0.f, 0.f), // N
            FVector(-1.f,  0.f, 0.f), // S
            FVector(0.f,  1.f, 0.f), // E
            FVector(0.f, -1.f, 0.f), // W
        };

        const int32 SafeIndex = FMath::RandRange(0, 3);
        const FVector SafeDirection = Cardinals[SafeIndex];

        SafeZoneCenter = BossLocation + SafeDirection * RowSnapshot.SafeZoneDistance;

    }

    {
        USkeletalMeshComponent* BossMesh = Boss->GetMesh();
        FVector CastLoc = (IsValid(BossMesh) && BossMesh->DoesSocketExist(ProjectileSpawnSocket))
            ? BossMesh->GetSocketLocation(ProjectileSpawnSocket)
            : Boss->GetActorLocation();

        MulticastSpawnAreaFX(
            CastLoc, RowSnapshot.bHasDirectionalSafeZone,
            SafeZoneCenter, RowSnapshot.AreaCastEffect.Get(),
            RowSnapshot.SafeZoneEffect.Get(),
            RowSnapshot.SafeZoneRadius,
            RowSnapshot.AreaAttackDelay
        );
    }

    const FPTBossSkillRow Snapshot = RowSnapshot;
    AreaAttackTimers.SetNum(Count);

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("[SpawnArea] AreaAttackRadius: %.1f"), Snapshot.AreaAttackRadius);
#endif

    MulticastSpawnAreaWarningBatch(
        DropLocations,
        Snapshot.AreaAttackDelay,
        Snapshot.AreaAttackInterval,
        Snapshot.AreaFallEffect.Get(),
        Snapshot.AreaImpactEffect.Get(),
        Snapshot.AreaStartHeight,
        Snapshot.AreaWarningClass,
        Snapshot.AreaAttackRadius,
        Snapshot.bGroundMode
    );

    bAreaAttackInProgress = true;

    for (int32 i = 0; i < Count; ++i)
    {
        const float Delay = Snapshot.AreaAttackDelay + Snapshot.AreaAttackInterval * i;
        const bool bIsFirst = (i == 0);
        const bool bIsLast = (i == Count - 1);

        World->GetTimerManager().SetTimer(
            AreaAttackTimers[i],
            FTimerDelegate::CreateWeakLambda(this, [this, Boss, DropLoc = DropLocations[i], Snapshot, SafeZoneCenter, bIsFirst, bIsLast]()
                {
                    UWorld* InnerWorld = GetWorld();
                    if (!IsValid(InnerWorld) || !IsValid(Boss))
                    {
                        if (bIsLast)
                        {
                            bAreaAttackInProgress = false;
                        }

                        return;
                    }

                    TArray<FHitResult>    HitResults;
                    TSet<AActor*>         LocalHitActors;
                    FCollisionQueryParams Params;
                    Params.AddIgnoredActor(Boss);

                    InnerWorld->SweepMultiByChannel(
                        HitResults, DropLoc, DropLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Snapshot.AreaAttackRadius), Params);

                    const float FinalDamage = Boss->GetBaseAtk() * Snapshot.DamageMultiplier * Boss->GetDamageMultiplierForPhase(Boss->GetCurrentPhase());

                    for (const FHitResult& Hit : HitResults)
                    {
                        AActor* HitActor = Hit.GetActor();
                        if (!IsValid(HitActor) || LocalHitActors.Contains(HitActor))
                        {
                            continue;
                        }

                        LocalHitActors.Add(HitActor);

                        if (Snapshot.bHasDirectionalSafeZone)
                        {
                            const float DistToSafe = FVector::Dist2D(HitActor->GetActorLocation(), SafeZoneCenter);
                            if (DistToSafe <= Snapshot.SafeZoneRadius)
                            {
                                continue;
                            }
                        }

                        if (APTBaseCharacter* Victim = Cast<APTBaseCharacter>(HitActor))
                        {
                            FPTHitInfo HitInfo = Snapshot.MakeHitInfo(Boss);
                            HitInfo.HitDirection = (HitActor->GetActorLocation() - DropLoc).GetSafeNormal();
                            Victim->ApplyDamageWithHit(FinalDamage, Boss, HitInfo);
                        }
                    }

                    if (bIsLast)
                    {
                        bAreaAttackInProgress = false;
                        MulticastStopSafeZoneFX();

                        if (Snapshot.bHoldMontageUntilDelay)
                        {
                            if (USkeletalMeshComponent* BossMesh = Boss->GetMesh())
                            {
                                if (UAnimInstance* AnimInstance = BossMesh->GetAnimInstance())
                                {
                                    AnimInstance->Montage_Resume(nullptr);
                                }
                            }
                        }
                    }
                }),
            Delay, false);
    }
}

void UPTBossPatternComponent::SpawnLaser(const FPTBossSkillRow& RowSnapshot)
{
    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner());
    if (!IsValid(Boss) || !Boss->HasAuthority())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    ClearLaserTimers();
    StopLaserFXLocal();

    bIsLaserActive = true;
    bLaserPhaseActive = true;

    if (UCharacterMovementComponent* MoveComp = Boss->GetCharacterMovement())
    {
        if (MoveComp->MaxWalkSpeed > 0.f)
        {
            SavedMaxWalkSpeed = MoveComp->MaxWalkSpeed;
        }

        MoveComp->MaxWalkSpeed = 0.f;
        MoveComp->bOrientRotationToMovement = false;
        MoveComp->bUseControllerDesiredRotation = false;
    }

    if (AAIController* AIC = Cast<AAIController>(Boss->GetController()))
    {
        AIC->StopMovement();
        AIC->ClearFocus(EAIFocusPriority::Gameplay);
    }

    MulticastPauseMontage();

    AActor* Target = GetTargetActor();
    const FVector LaserStart = Boss->GetActorLocation()
        + Boss->GetActorForwardVector() * 120.f
        + FVector(0.f, 0.f, 50.f);

    LaserFireDirection = Boss->GetActorForwardVector();

    UNiagaraSystem* LaserFX = RowSnapshot.LaserEffect.Get();

    const float TotalTicks = FMath::Max(1.f, RowSnapshot.LaserDuration / FMath::Max(0.05f, RowSnapshot.LaserTickInterval));
    const float BaseDamage = Boss->GetBaseAtk() * RowSnapshot.DamageMultiplier * Boss->GetDamageMultiplierForPhase(Boss->GetCurrentPhase());
    const float DamagePerTick = BaseDamage / TotalTicks;
    const FPTBossSkillRow Snapshot = RowSnapshot;
    const FVector InitialMaxEnd = LaserStart + LaserFireDirection * Snapshot.LaserRange;

    FHitResult InitialWallHit;
    FCollisionQueryParams InitialParams;
    InitialParams.AddIgnoredActor(Boss);
    const bool bInitialWallHit = World->LineTraceSingleByChannel(
        InitialWallHit, LaserStart, InitialMaxEnd, ECC_Visibility, InitialParams);
    const FVector InitialEnd = bInitialWallHit ? InitialWallHit.ImpactPoint : InitialMaxEnd;
    const float InitialDist = FVector::Dist(LaserStart, InitialEnd);
    MulticastStartLaserFX(LaserFireDirection, InitialDist, LaserFX, RowSnapshot.LaserSocketName, RowSnapshot.LaserLoopSound.Get(), RowSnapshot.LaserSoundDelay);

    World->GetTimerManager().SetTimer(
        LaserTickTimerHandle,
        FTimerDelegate::CreateWeakLambda(this, [this, Boss, Snapshot, DamagePerTick]()
            {
                UWorld* InnerWorld = GetWorld();
                if (!IsValid(InnerWorld) || !IsValid(Boss))
                {
                    return;
                }

                const FVector Start = Boss->GetActorLocation()
                    + Boss->GetActorForwardVector() * 120.f
                    + FVector(0.f, 0.f, 50.f);
                const FVector End = Start + LaserFireDirection * Snapshot.LaserRange;
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(Boss);

                FHitResult WallHit;
                const bool bWallHit = InnerWorld->LineTraceSingleByChannel(
                    WallHit, Start, End, ECC_Visibility, Params);
                const FVector EffectiveEnd = bWallHit ? WallHit.ImpactPoint : End;

                TArray<FHitResult> PawnHits;
                InnerWorld->LineTraceMultiByChannel(PawnHits, Start,
                    EffectiveEnd, ECC_Pawn, Params);

                TSet<APTBaseCharacter*> DamagedThisTick;

                for (const FHitResult& Hit : PawnHits)
                {
                    APTBaseCharacter* TargetChar = Cast<APTBaseCharacter>(Hit.GetActor());
                    if (!IsValid(TargetChar))
                    {
                        continue;
                    }

                    if (DamagedThisTick.Contains(TargetChar))
                    {
                        continue;
                    }

                    DamagedThisTick.Add(TargetChar);

                    FPTHitInfo HitInfo = Snapshot.MakeHitInfo(Boss);
                    HitInfo.HitDirection = LaserFireDirection;
                    TargetChar->ApplyDamageWithHit(DamagePerTick, Boss, HitInfo);
                }

                const float EffectiveDist = FVector::Dist(Start, EffectiveEnd);
                MulticastUpdateLaserFX(EffectiveDist);
            }),
        Snapshot.LaserTickInterval, /*bLooping=*/true);

    World->GetTimerManager().SetTimer(
        LaserEndTimerHandle,
        FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                StopLaser();
            }),
        Snapshot.LaserDuration, /*bLooping=*/false);
}

void UPTBossPatternComponent::MulticastPauseMontage_Implementation()
{
    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner());
    if (!IsValid(Boss))
    {
        return;
    }

    if (USkeletalMeshComponent* BossMesh = Boss->GetMesh())
    {
        if (UAnimInstance* AnimInstance = BossMesh->GetAnimInstance())
        {
            AnimInstance->Montage_Pause(nullptr);
        }
    }
}

void UPTBossPatternComponent::MulticastResumeMontage_Implementation()
{
    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner());
    if (!IsValid(Boss))
    {
        return;
    }

    if (USkeletalMeshComponent* BossMesh = Boss->GetMesh())
    {
        if (UAnimInstance* AnimInstance = BossMesh->GetAnimInstance())
        {
            AnimInstance->Montage_Resume(nullptr);
        }
    }
}

void UPTBossPatternComponent::StopSafeZoneFXLocal()
{
    if (IsValid(ActiveSafeZoneComp))
    {
        ActiveSafeZoneComp->DeactivateImmediate();
        ActiveSafeZoneComp->DestroyComponent();
        ActiveSafeZoneComp = nullptr;
    }
}

void UPTBossPatternComponent::StopLaserFXLocal()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LaserSoundDelayHandle);
    }

    if (IsValid(ActiveLaserComponent))
    {
        ActiveLaserComponent->Deactivate();
        ActiveLaserComponent->DestroyComponent();
        ActiveLaserComponent = nullptr;
    }

    if (IsValid(ActiveLaserAudioComp))
    {
        ActiveLaserAudioComp->Stop();
        ActiveLaserAudioComp = nullptr;
    }
}

void UPTBossPatternComponent::MulticastStartLaserFX_Implementation(FVector FireDirection, float InitialDist, UNiagaraSystem* LaserFX, FName SocketName, USoundBase* LaserSound, float SoundDelay)
{
    StopLaserFXLocal();

    if (!IsValid(LaserFX))
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    LaserFireDirection = FireDirection;
    bUseMeleeBeamVar = !SocketName.IsNone();

    FVector Start;
    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetOwner());
    USkeletalMeshComponent* BossMesh = IsValid(Boss) ? Boss->GetMesh() : nullptr;

    if (!SocketName.IsNone() && IsValid(BossMesh) && BossMesh->DoesSocketExist(SocketName))
    {
        Start = BossMesh->GetSocketLocation(SocketName);
    }
    else
    {
        Start = IsValid(Boss) ? Boss->GetActorLocation() + Boss->GetActorForwardVector() * 120.f + FVector(0.f, 0.f, 50.f) : FVector::ZeroVector;
    }

    const FRotator SpawnRot = SocketName.IsNone() ? FRotator::ZeroRotator : FireDirection.Rotation();

    ActiveLaserComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World, LaserFX, Start, SpawnRot,
        FVector::OneVector, false, true);

    if (IsValid(ActiveLaserComponent) && SocketName.IsNone())
    {
        ActiveLaserComponent->SetNiagaraVariableVec3(TEXT("beamEnd"), FireDirection * InitialDist);
    }

    if (!IsValid(Boss) || !IsValid(LaserSound))
    {
        return;
    }

    if (SoundDelay <= 0.f)
    {
        ActiveLaserAudioComp = UGameplayStatics::SpawnSoundAttached(
            LaserSound, Boss->GetRootComponent(), NAME_None,
            FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true
        );
    }
    else
    {
        TWeakObjectPtr<UPTBossPatternComponent> WeakThis(this);
        USoundBase* CapturedSound = LaserSound;
        USceneComponent* CapturedRoot = Boss->GetRootComponent();
        World->GetTimerManager().SetTimer(LaserSoundDelayHandle, [WeakThis, CapturedSound, CapturedRoot]()
            {
                if (!WeakThis.IsValid() || !IsValid(CapturedSound) || !IsValid(CapturedRoot))
                {
                    return;
                }

                WeakThis->ActiveLaserAudioComp = UGameplayStatics::SpawnSoundAttached(
                    CapturedSound, CapturedRoot, NAME_None,
                    FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true);
            }, SoundDelay, false);
    }
}

void UPTBossPatternComponent::MulticastUpdateLaserFX_Implementation(float EffectiveDist)
{
    if (!IsValid(ActiveLaserComponent))
    {
        return;
    }

    if (!bUseMeleeBeamVar)
    {
        ActiveLaserComponent->SetNiagaraVariableVec3(TEXT("beamEnd"), LaserFireDirection * EffectiveDist);
    }
}

void UPTBossPatternComponent::MulticastStopLaserFX_Implementation()
{
    StopLaserFXLocal();
}

AActor* UPTBossPatternComponent::GetTargetActor() const
{
    const APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(GetOwner());
    if (!IsValid(Monster))
    {
        return nullptr;
    }

    const AAIController* AIC = Cast<AAIController>(Monster->GetController());
    if (!IsValid(AIC))
    {
        return nullptr;
    }

    const UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return nullptr;
    }

    return Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
}
