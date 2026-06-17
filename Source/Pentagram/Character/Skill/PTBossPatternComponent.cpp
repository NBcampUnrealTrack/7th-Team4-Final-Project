#include "Character/Skill/PTBossPatternComponent.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Character/Monsters/Projectile/PTBossProjectile.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "Character/Skill/PTMonsterSkillComponent.h"
#include "Character/Skill/PTSkillComponent.h"
#include "Character/PTBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DataTable.h"

UPTBossPatternComponent::UPTBossPatternComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPTBossPatternComponent::BeginPlay()
{
    Super::BeginPlay();

    if (APTMonsterCharacter* Owner = Cast<APTMonsterCharacter>(GetOwner()))
    {
        SkillComponent = Owner->SkillComponent;
    }
}

void UPTBossPatternComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (auto& Pair : PatternCooldownTimers)
        {
            World->GetTimerManager().ClearTimer(Pair.Value);
        }
        World->GetTimerManager().ClearTimer(AreaAttackTimer);
    }

    Super::EndPlay(EndPlayReason);
}

void UPTBossPatternComponent::PreloadAllSkills()
{
    if (!IsValid(BossSkillDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] BossSkillDataTable 미설정"));
        bSkillAssetsLoaded = false;
        return;
    }

    const int32 TotalRows = Phase0SkillRowNames.Num() + Phase1SkillRowNames.Num() + Phase2SkillRowNames.Num();
    if (TotalRows <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] Preload 실패 — Phase RowName 목록이 비어 있음"));
        bSkillAssetsLoaded = false;
        return;
    }

    int32 FailCount = 0;

    auto LoadRow = [&FailCount](FPTBossSkillRow* Row, const FName& RowName)
    {
        if (!Row)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BossPattern] Preload 실패 — [%s] RowName 또는 RowStruct 확인"), *RowName.ToString());
            ++FailCount;
            return;
        }

        Row->SkillMontage.LoadSynchronous();
        Row->OverrideMontage.LoadSynchronous();
        Row->SkillEffect.LoadSynchronous();
        Row->SkillSound.LoadSynchronous();
        Row->SkillHitSound.LoadSynchronous();
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
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] %d / %d Row 로드 실패 — 일부 스킬이 동작하지 않을 수 있음"),
            FailCount, TotalRows);
    }

    bSkillAssetsLoaded = (FailCount < TotalRows);
}

bool UPTBossPatternComponent::ExecuteSkillForPhase(int32 Phase)
{
    if (!bSkillAssetsLoaded)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] 스킬 에셋 미로드 — PreloadAllSkills 호출 확인"));
        return false;
    }

    if (!IsValid(SkillComponent))
    {
        return false;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return false;
    }

    bHasPendingSkill = false;

    auto [RowName, Row] = PickNextSkill(Phase);
    if (!Row)
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("[BossPattern] 선택 성공 — %s"), *RowName.ToString());

    FPTSkillActivationRequest Request;
    Request.SkillRowName    = RowName;
    Request.SkillDataTable  = BossSkillDataTable;
    Request.OverrideMontage = Row->OverrideMontage;

    const bool bActivated = SkillComponent->TryActivateSkillChecked(Request);
    if (!bActivated)
    {
        return false;
    }

    PendingSkillSnapshot = *Row;
    bHasPendingSkill     = true;
    UE_LOG(LogTemp, Log, TEXT("[BossPattern] Pending Skill Set: %s"), *RowName.ToString());

    if (Row->PatternCooldown <= 0.f)
    {
        return true;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return true;
    }

    PatternCooldownFlags.Add(RowName, true);
    FTimerHandle& Timer = PatternCooldownTimers.FindOrAdd(RowName);

    UE_LOG(LogTemp, Log, TEXT("[BossPattern] PatternCooldown 시작 — %s (%.1f초)"),
        *RowName.ToString(), Row->PatternCooldown);

    World->GetTimerManager().SetTimer(Timer,
        [this, RowName]()
        {
            PatternCooldownFlags.Add(RowName, false);
            UE_LOG(LogTemp, Log, TEXT("[BossPattern] PatternCooldown 종료 — %s"), *RowName.ToString());
        },
        Row->PatternCooldown, false);

    return true;
}

void UPTBossPatternComponent::ExecutePendingSkill(int32 Phase)
{
    if (!bHasPendingSkill)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossPattern] ExecutePendingSkill 호출됐지만 대기 중인 스킬 없음 — 몽타주 노티파이 확인"));
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

    default:
        break;
    }
}

TPair<FName, FPTBossSkillRow*> UPTBossPatternComponent::PickNextSkill(int32 Phase)
{
    if (!IsValid(BossSkillDataTable))
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
            UE_LOG(LogTemp, Warning, TEXT("[BossPattern] [%s] Row 조회 실패 — 후보에서 제외"), *RowName.ToString());
            continue;
        }

        if (Row->Weight <= 0.f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BossPattern] [%s] Weight <= 0 — 후보에서 제외"), *RowName.ToString());
            continue;
        }

        if (PatternCooldownFlags.FindRef(RowName))
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
    float Acc  = 0.f;
    for (auto& [RowName, Row] : Candidates)
    {
        Acc += Row->Weight;
        if (Rand <= Acc)
        {
            return { RowName, Row };
        }
    }

    return { NAME_None, nullptr };
}

void UPTBossPatternComponent::SpawnProjectile(const FPTBossSkillRow& RowSnapshot)
{
    if (!IsValid(ProjectileClass))
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
    SpawnParams.Owner      = Boss;
    SpawnParams.Instigator = Boss;

    APTBossProjectile* Projectile = World->SpawnActor<APTBossProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (!IsValid(Projectile))
    {
        return;
    }

    AActor* Target = GetTargetActor();
    FVector Direction = IsValid(Target) ? (Target->GetActorLocation() - SpawnLocation).GetSafeNormal() : Boss->GetActorForwardVector();

    const float FinalDamage = Boss->GetBaseAtk() * RowSnapshot.DamageMultiplier * Boss->GetDamageMultiplierForPhase(Boss->GetCurrentPhase());

    FPTHitInfo HitInfo   = RowSnapshot.MakeHitInfo(Boss);

    Projectile->Launch(Direction, FinalDamage, RowSnapshot.ProjectileSpeed, HitInfo);
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

    const FVector TargetLocation = Target->GetActorLocation();

    // TODO : 바닥에 이펙트

    const FPTBossSkillRow Snapshot = RowSnapshot;

    World->GetTimerManager().SetTimer(
        AreaAttackTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, Boss, TargetLocation, Snapshot]()
            {
                UWorld* InnerWorld = GetWorld();
                if (!IsValid(InnerWorld) || !IsValid(Boss))
                {
                    return;
                }

                TArray<FHitResult> HitResults;
                TSet<AActor*> LocalHitActors;
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(Boss);

                InnerWorld->SweepMultiByChannel(
                    HitResults, TargetLocation, TargetLocation, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Snapshot.AreaAttackRadius), Params);

                const float FinalDamage = Boss->GetBaseAtk() * Snapshot.DamageMultiplier * Boss->GetDamageMultiplierForPhase(Boss->GetCurrentPhase());

                for (const FHitResult& Hit : HitResults)
                {
                    AActor* HitActor = Hit.GetActor();
                    if (!IsValid(HitActor) || LocalHitActors.Contains(HitActor))
                    {
                        continue;
                    }

                    LocalHitActors.Add(HitActor);

                    if (APTBaseCharacter* Victim = Cast<APTBaseCharacter>(HitActor))
                    {
                        FPTHitInfo HitInfo = Snapshot.MakeHitInfo(Boss);
                        HitInfo.HitDirection = (HitActor->GetActorLocation() - TargetLocation).GetSafeNormal();
                        Victim->ApplyDamageWithHit(FinalDamage, Boss, HitInfo);
                    }
                }
            }),
        RowSnapshot.AreaAttackDelay, false);
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
