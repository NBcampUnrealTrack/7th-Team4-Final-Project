#include "Character/PTBaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerCharacter.h" 
#include "Player/PTEquipmentComponent.h"

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

    // 만약 때린 놈(Attacker)이 존재하고, 그 놈이 플레이어 캐릭터라면?
    if (APTPlayerCharacter* AttackerPlayer = Cast<APTPlayerCharacter>(Attacker))
    {
        FinalDamageAmount = AttackerPlayer->GetTotalAttack() * DamageAmount;
    }

    // [데미지 계산 공식] 기존 DamageAmount 대신 장비 스탯이 합산된 FinalDamageAmount를 사용
    float FinalDamage = FMath::Max(FinalDamageAmount - BaseDef, 1.f);

    // HP 감소
    CurrentHP = FMath::Max(CurrentHP - FinalDamage, 0.f);

    // PlayerState에 결과 반영 (플레이어만)
    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentHP = CurrentHP;
        PS->MaxHP = MaxHP;
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
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
}

void APTBaseCharacter::OnDeath()
{
    // 이동 불가
    GetCharacterMovement()->DisableMovement();

    // 콜리전 비활성화 (액터끼리 충돌 안함, 나중에 리스폰 시에 활성화 시켜줘야 할 수 있음.)
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 사망 애니메이션 재생은 각 파생 클래스에서 구현해주세요.
}

void APTBaseCharacter::OnRep_CurrentHP()
{
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
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
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

        CachedWalkSpeed       = Movement->MaxWalkSpeed;
        bCachedOrientRotation = Movement->bOrientRotationToMovement;

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
        PlayAnimMontage(HitReaction_Light);
    }
    else
    {
        LaunchCharacter(Dir * HitInfo.KnockbackForce + FVector::UpVector * HitInfo.KnockbackZForce, true, true);
        PlayAnimMontage(HitReaction_Heavy);
    }
}
