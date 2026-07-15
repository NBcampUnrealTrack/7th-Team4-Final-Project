#include "Character/PTBaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "UI/Widget/Widget/Player/PTDamageNumberWidget.h"
#include "Player/PTPlayerController.h"

APTBaseCharacter::APTBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

float APTBaseCharacter::ApplyDamage(float DamageAmount, AActor* Attacker)
{
    if (!HasAuthority()) return 0.f;

    // 플레이어가 닷지 상태일 때 무적 판정을 체크함
    if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(this))
    {
        if (Player->bIsInvincible) return 0.f;
    }

    if (Cast<APTPlayerCharacter>(this) && Cast<APTPlayerCharacter>(Attacker))
    {
        return 0.f;
    }

    // 기본 데미지는 DamageAmount로 시작 (때린 놈의 장비 스탯이 있다면 그걸 더해줘야 함)
    float FinalDamageAmount = DamageAmount;

    bool bIsCritical = false;

    // 만약 때린 놈(Attacker)이 존재하고, 그 놈이 플레이어 캐릭터라면?
    if (APTPlayerCharacter* AttackerPlayer = Cast<APTPlayerCharacter>(Attacker))
    {
        FinalDamageAmount = AttackerPlayer->GetTotalAttack() * DamageAmount;

        // 크리티컬 판정 (BeginPlay에서 Row로부터 로드된 멤버 변수 사용 - DataTable 재조회 불필요)
        if (FMath::FRand() < AttackerPlayer->CriticalChance)
        {
            float CritMult = (AttackerPlayer->CriticalATK > 0.f) ? AttackerPlayer->CriticalATK : 2.f;
            FinalDamageAmount *= CritMult;
            bIsCritical = true;
        }
    }

    // [데미지 계산 공식] 기존 DamageAmount 대신 장비 스탯이 합산된 FinalDamageAmount를 사용
    float FinalDamage = FMath::Max(FinalDamageAmount - BaseDef, 1.f);

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("[ApplyDamage] Input=%.1f BaseDef=%.1f Final=%.1f"),
        FinalDamageAmount, BaseDef, FinalDamage);
#endif

    if (APTPlayerCharacter* HitPlayer = Cast<APTPlayerCharacter>(this))
    {
        if (APTPlayerController* HitPC = Cast<APTPlayerController>(HitPlayer->GetController()))
        {
            FVector HitDisplayLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f);
            HitPC->Client_ShowDamageNumber(HitDisplayLocation, FinalDamage, bIsCritical);
        }
    }
    else if (APTPlayerCharacter* AttackerPlayer = Cast<APTPlayerCharacter>(Attacker))
    {
        if (APTPlayerController* AttackerPC = Cast<APTPlayerController>(AttackerPlayer->GetController()))
        {
            FVector HitDisplayLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f);
            AttackerPC->Client_ShowDamageNumber(HitDisplayLocation, FinalDamage, bIsCritical);
        }
    }

    // HP 감소
    CurrentHP = FMath::Max(CurrentHP - FinalDamage, 0.f);

    // PlayerState에 결과 반영 (플레이어만)
    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentHP = CurrentHP;
        PS->MaxHP = MaxHP;

        PS->OnHealthChanged.Broadcast(PS->CurrentHP, PS->MaxHP);
    }

    // 죽음
    if (CurrentHP <= 0.f)
    {
        OnDeath();
    }

    return FinalDamage;
}

float APTBaseCharacter::ApplyDamageWithHit(float DamageAmount, AActor* Attacker, const FPTHitInfo& HitInfo)
{
    if (CurrentHP <= 0.f)
    {
        return 0.f;
    }

    const float FinalDamage = ApplyDamage(DamageAmount, Attacker);

    if (FinalDamage <= 0.f)
    {
        return 0.f;
    }

    if (CurrentHP <= 0.f)
    {
        ApplyHitStop(HitInfo.HitStopDuration);
        return FinalDamage;
    }

    ApplyHit(HitInfo);
    return FinalDamage;
}

void APTBaseCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (const FPTCharacterRow* Row = CharacterDataHandle.GetRow<FPTCharacterRow>(TEXT("Load")))
    {
        MaxHP = Row->MaxHP;         CurrentHP = Row->MaxHP;
        MaxMP = Row->MaxMP;         CurrentMP = Row->MaxMP;
        BaseDef = Row->BaseDef;     BaseAtk = Row->BaseAtk;
        AttackSpeed = Row->AttackSpeed;
        MoveSpeed = Row->MoveSpeed;
        CriticalChance = Row->CriticalChance;
        CriticalATK = Row->CriticalATK;
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }

    // PostInitializeComponents 시점엔 PlayerState가 아직 없을 수 있어(특히 클라이언트)
    // 실패해도 무해함 - BeginPlay에서 한 번 더 시도함
    SyncCombatStatsToPlayerState();
}

void APTBaseCharacter::OnDeath()
{
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

bool APTBaseCharacter::IsDead() const
{
    return CurrentHP <= 0.f;
}

void APTBaseCharacter::OnRep_CurrentHP()
{
}

void APTBaseCharacter::RequestHitStop(float Duration)
{
    ApplyHitStop(Duration);
}

void APTBaseCharacter::SyncCombatStatsToPlayerState()
{
    if (!HasAuthority())
    {
        return;
    }

    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (!PS)
    {
        return;
    }

    PS->BaseAtk = BaseAtk;
    PS->BaseDef = BaseDef;
    PS->CriticalChance = CriticalChance;
    PS->CriticalATK = CriticalATK;
    PS->MoveSpeed = MoveSpeed;

    // 서버 자신(리슨서버 호스트)의 UI는 OnRep이 안 불리므로 직접 브로드캐스트
    PS->OnAttackChanged.Broadcast(PS->BaseAtk);
    PS->OnDefenseChanged.Broadcast(PS->BaseDef);
    PS->OnCriticalChanged.Broadcast(PS->CriticalChance, PS->CriticalATK);
    PS->OnMoveSpeedChanged.Broadcast(PS->MoveSpeed);
}

void APTBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // DT에서 캐릭터 스탯을 로드
    if (const FPTCharacterRow* Row = CharacterDataHandle.GetRow<FPTCharacterRow>(TEXT("BeginPlay")))
    {
        MaxHP = Row->MaxHP;
        CurrentHP = Row->MaxHP;
        MaxMP = Row->MaxMP;
        CurrentMP = Row->MaxMP;
        BaseDef = Row->BaseDef;
        BaseAtk = Row->BaseAtk;
        AttackSpeed = Row->AttackSpeed;
        MoveSpeed = Row->MoveSpeed;
        CriticalChance = Row->CriticalChance;
        CriticalATK = Row->CriticalATK;
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }

    // BeginPlay 시점엔 대부분 PossessedBy가 끝난 뒤라 PlayerState 확보돼있음
    SyncCombatStatsToPlayerState();
}

void APTBaseCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTBaseCharacter, CurrentHP);
    DOREPLIFETIME(APTBaseCharacter, MaxHP);
    DOREPLIFETIME(APTBaseCharacter, CurrentMP);
    DOREPLIFETIME(APTBaseCharacter, MaxMP);
    DOREPLIFETIME(APTBaseCharacter, BaseDef);
    DOREPLIFETIME(APTBaseCharacter, BaseAtk);
    DOREPLIFETIME(APTBaseCharacter, AttackSpeed);
    DOREPLIFETIME(APTBaseCharacter, MoveSpeed);
    DOREPLIFETIME(APTBaseCharacter, CriticalChance);
    DOREPLIFETIME(APTBaseCharacter, CriticalATK);
}

void APTBaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    SyncCombatStatsToPlayerState();
}

void APTBaseCharacter::ApplyHit(const FPTHitInfo& HitInfo)
{
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    ApplyHitStop(HitInfo.HitStopDuration);

    if (APTBaseCharacter* AttackerChar = Cast<APTBaseCharacter>(HitInfo.Attacker))
    {
        AttackerChar->ApplyHitStop(HitInfo.HitStopDuration * 0.5f);
    }

    if (HasAuthority())
    {
        Multicast_PlayHitReactionMontage(HitInfo.HitReactionType);
    }

    if (HitInfo.KnockbackForce > 0.f)
    {
        ApplyKnockback(HitInfo);
    }

    if (HitInfo.StaggerDuration > 0.f)
    {
        UCharacterMovementComponent* Movement = GetCharacterMovement();
        if (!IsValid(Movement))
        {
            return;
        }

        if (!bIsStaggered)
        {
            CachedWalkSpeed = Movement->MaxWalkSpeed;
            bCachedOrientRotation = Movement->bOrientRotationToMovement;
        }

        bIsStaggered = true;
        Movement->MaxWalkSpeed = 0.f;
        Movement->bOrientRotationToMovement = false;

        World->GetTimerManager().ClearTimer(StaggerTimer);
        World->GetTimerManager().SetTimer(
            StaggerTimer,
            [this]()
            {
                bIsStaggered = false;
                if (UCharacterMovementComponent* M = GetCharacterMovement())
                {
                    M->MaxWalkSpeed              = CachedWalkSpeed;
                    M->bOrientRotationToMovement = bCachedOrientRotation;
                }
            },
            HitInfo.StaggerDuration, false);
    }
}

void APTBaseCharacter::ApplyHitStop(float Duration)
{
    if (Duration <= 0.f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    World->GetTimerManager().ClearTimer(HitStopTimer);
    CustomTimeDilation = 0.05f;
    World->GetTimerManager().SetTimer(
        HitStopTimer, this, &APTBaseCharacter::RestoreHitStop,
        Duration, false);
}

void APTBaseCharacter::RestoreHitStop()
{
    CustomTimeDilation = DefaultTimeDilation;
}

void APTBaseCharacter::ApplyKnockback(const FPTHitInfo& HitInfo)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!IsValid(Movement))
    {
        return;
    }

    FVector Dir = HitInfo.HitDirection.GetSafeNormal();
    if (Dir.IsNearlyZero())
    {
        Dir = GetActorForwardVector();
    }

    if (HitInfo.HitReactionType == EHitReactionType::Light)
    {
        Movement->AddImpulse(Dir * HitInfo.KnockbackForce, true);
    }
    else
    {
        LaunchCharacter(Dir * HitInfo.KnockbackForce + FVector::UpVector * HitInfo.KnockbackZForce, true, true);
        PlayAnimMontage(HitReaction_Heavy);
    }
}

void APTBaseCharacter::Multicast_PlayHitReactionMontage_Implementation(EHitReactionType ReactionType)
{
    // 몽타주 호출 확인용 로그 — 개발 확인용, Shipping 제외
#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("Hit Montage Multicast Called"));
#endif

    UAnimMontage* Montage = (ReactionType == EHitReactionType::Light)
        ? HitReaction_Light
        : HitReaction_Heavy;

    if (!IsValid(Montage))
    {
        // 몽타주 미설정 감지용 — 개발 중 잡을 문제, Shipping 제외
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("Hit Montage is null"));
#endif
        return;
    }

    PlayAnimMontage(Montage);
}
