#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "PTMonsterState.h"
#include "PTMonsterCharacter.generated.h"

class UAnimMontage;
class APTGoldPickup;
class APTBasePlayerState;

UCLASS()
class PENTAGRAM_API APTMonsterCharacter : public APTBaseCharacter
{
	GENERATED_BODY()

public:
    APTMonsterCharacter();

    virtual float ApplyDamage(float DamageAmount, AActor* Attacker) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnDeath() override;
    virtual float GetAttackDamage() const;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentState, VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    EMonsterState CurrentState = EMonsterState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    FVector SpawnLocation = FVector::ZeroVector;

public:
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

    void PerformAttack();
    virtual float StartAttack();
    virtual void StopAttack();

    UFUNCTION()
    void OnRep_CurrentState();

protected:
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

    TSet<TWeakObjectPtr<APTBasePlayerState>> ExpContributors;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Monster")
    float DestroyDelay = 3.f;

    FTimerHandle DestroyTimerHandle;

    void RegisterDamageContributor(AActor* DamageCauser);
    void GiveExpToContributors();
    void SpawnDeathDrops();
    void HandleDestroyAfterDeath();
    void PlayDeathMontage();
};
