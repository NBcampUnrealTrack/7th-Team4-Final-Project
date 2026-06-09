#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "PTMonsterState.h"
#include "PTMonsterRewardData.h"
#include "UI/Data/PTDelegates.h"
#include "PTMonsterCharacter.generated.h"

class UAnimMontage;
struct FDataTableRowHandle;
class APTBasePlayerState;
class APTGoldPickup;

UCLASS()
class PENTAGRAM_API APTMonsterCharacter : public APTBaseCharacter
{
	GENERATED_BODY()

public:
    APTMonsterCharacter();

    virtual float ApplyDamage(float DamageAmount, AActor* Attacker) override;

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

    void PerformAttack();
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

protected:
    UFUNCTION()
    void OnRep_CurrentState();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnDeath() override;
    virtual float GetAttackDamage() const;
    virtual void OnRep_CurrentHP() override;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentState, VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    EMonsterState CurrentState = EMonsterState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY()
    TSet<AActor*> HitActors;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Animation")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster|Animation")
    TObjectPtr<UAnimMontage> DeathMontage;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float SightAngle = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float SightRange = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float ChaseRange = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster|AI", meta = (AllowPrivateAccess = "true"))
    float AttackRange = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Monster|Combat", meta = (AllowPrivateAccess = "true"))
    float AttackForwardOffset = 150.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Monster|Combat", meta = (AllowPrivateAccess = "true"))
    float AttackHeightOffset = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PT|Monster|Combat", meta = (AllowPrivateAccess = "true"))
    float AttackRadius = 150.f;

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

    FTimerHandle DestroyTimerHandle;

    void RegisterDamageContributor(AActor* DamageCauser);
    void DestroyAfterDeath();
    float PlayDeathMontage();
};
