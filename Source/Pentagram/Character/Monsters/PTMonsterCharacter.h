#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Monsters/PTMonsterState.h"
#include "Character/Monsters/PTMonsterRewardData.h"
#include "UI/Data/PTDelegates.h"
#include "PTMonsterCharacter.generated.h"

class UAnimMontage;
struct FDataTableRowHandle;
class APTBasePlayerState;
class APTGoldPickup;
class UPTMonsterSkillComponent;
class APTBossProjectile;

UCLASS()
class PENTAGRAM_API APTMonsterCharacter : public APTBaseCharacter
{
	GENERATED_BODY()

public:
    APTMonsterCharacter();

    UFUNCTION(BlueprintPure, Category = "PT|Monster")
    FText GetMonsterDisplayName() const { return MonsterDisplayName; }

    virtual float ApplyDamage(float DamageAmount, AActor* Attacker) override;

    virtual float ApplyDamageWithHit(float DamageAmount, AActor* Attacker, const FPTHitInfo& HitInfo) override;

    UFUNCTION(BlueprintCallable, Category = "PT|Monster")
    void InitializeMonster();

    UFUNCTION(BlueprintCallable, Category = "PT|Monster")
    void SetMonsterState(EMonsterState NewState);

    float   GetSightAngle()         const { return SightAngle; }
    float   GetSightRange()         const { return SightRange; }
    float   GetChaseRange()         const { return ChaseRange; }
    float   GetAttackRange()        const { return AttackRange; }
    float   GetPatrolRadius()       const { return PatrolRadius; }
    float   GetMaxChaseDistance()   const { return MaxChaseDistance; }
    FVector GetSpawnLocation()      const { return SpawnLocation; }
    EMonsterState GetCurrentState() const { return CurrentState; }
    bool    IsDead()                const { return CurrentState == EMonsterState::Dead; }
    const TSet<TWeakObjectPtr<APTBasePlayerState>>& GetExpContributors() const { return ExpContributors; }

    UFUNCTION(BlueprintPure, Category = "PT|Monster|Combat")
    bool IsRangedMonster() const { return bIsRanged; }

    UFUNCTION(BlueprintPure, Category = "PT|Monster|Combat")
    float GetOptimalRange() const { return OptimalRangeValue; }

    virtual void PerformAttack();
    virtual float StartAttack();
    virtual void StopAttack();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayAttackMontage(UAnimMontage* MontageToPlay);

    /**
     * 몬스터의 보상 데이터를 스냅샷으로 반환합니다.
     * Destroy 후에도 안전하게 참조할 수 있습니다.
     */
    FPTMonsterRewardData GetRewardData() const;

    void ClearExpContributors();

    UPROPERTY(BlueprintAssignable, Category = "PT|Monster|UI")
    FPTOnBossHealthChanged OnHPChanged;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|Combat")
    TObjectPtr<UPTMonsterSkillComponent> SkillComponent;

    void SetSuperArmor(bool bEnable) { bHasSuperArmor = bEnable; }

    UPROPERTY(BlueprintAssignable, Category = "PT|Monster|Delegates")
    FPTOnMonsterDied OnMonsterDied;
protected:
    UFUNCTION()
    void OnRep_CurrentState();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnDeath() override;
    virtual float GetAttackDamage() const;
    virtual void OnRep_CurrentHP() override;

    virtual void Multicast_PlayHitReactionMontage_Implementation(EHitReactionType ReactionType) override;
    virtual void ApplyHit(const FPTHitInfo& HitInfo) override;

    void RestartBTAfterStagger(float Duration);
    void OnStaggerEnd();

    void ApplyAttackMovementLock();
    void RestoreAttackMovementLock();

    UPROPERTY(ReplicatedUsing = OnRep_CurrentState, VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    EMonsterState CurrentState = EMonsterState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(Transient)
    TSet<AActor*> HitActors;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Animation")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Animation")
    TObjectPtr<UAnimMontage> DeathMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Monster|Combat", meta = (AllowPrivateAccess = "true"))
    float AttackForwardOffset = 150.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Monster|Combat", meta = (AllowPrivateAccess = "true"))
    float AttackHeightOffset = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Monster|Combat", meta = (AllowPrivateAccess = "true"))
    float AttackRadius = 150.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Combat")
    bool bIsRanged = false;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Combat")
    float OptimalRangeValue = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Combat")
    TSubclassOf<APTBossProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Combat")
    FName ProjectileSocketName = TEXT("hand_r");

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Combat")
    float ProjectileSpeed = 1200.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Monster|Name", meta = (AllowPrivateAccess = "true"))
    FText MonsterDisplayName;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float SightAngle = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float SightRange = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float ChaseRange = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float AttackRange = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float PatrolRadius = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float MaxChaseDistance = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|Reward", meta = (AllowPrivateAccess = "true"))
    int32 GoldDropMin = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|Reward", meta = (AllowPrivateAccess = "true"))
    int32 GoldDropMax = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|Reward", meta = (AllowPrivateAccess = "true"))
    float EquipDropRate = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|Reward", meta = (AllowPrivateAccess = "true"))
    int32 RewardExp = 0;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Drop")
    TSubclassOf<APTGoldPickup> GoldPickupClass;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Drop")
    TSubclassOf<AActor> EquipmentDropClass;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Drop")
    FDataTableRowHandle ItemRowHandle;

    TSet<TWeakObjectPtr<APTBasePlayerState>> ExpContributors;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster")
    float DestroyDelay = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster")
    float DestroyDelayAfterMontage = 1.5f;

    bool bHasSuperArmor = false;
    bool bSavedOrientRotationToMovement = true;
    bool bAppliedMovementLock = false;

    FTimerHandle DestroyTimerHandle;
    FTimerHandle StaggerResumeTimerHandle;

    void RegisterDamageContributor(AActor* DamageCauser);
    void DestroyAfterDeath();
    float PlayDeathMontage();
};
