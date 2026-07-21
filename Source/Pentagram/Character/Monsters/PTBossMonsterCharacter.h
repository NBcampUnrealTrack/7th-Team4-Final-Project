#pragma once

#include "CoreMinimal.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "PTBossMonsterCharacter.generated.h"

class UPTBossPatternComponent;
class APTBossRoomCenter;
class APTBossShield;
class USoundBase;

UCLASS()
class PENTAGRAM_API APTBossMonsterCharacter : public APTMonsterCharacter
{
	GENERATED_BODY()

public:
    APTBossMonsterCharacter();

    virtual void PerformAttack() override;

    void SetMeleeAttackData(float Damage, const FPTHitInfo& HitInfo);
    void ClearMeleeAttackData();

    bool IsAlreadyHit(TWeakObjectPtr<AActor> Target) const;
    void AddHitActor(TWeakObjectPtr<AActor> Target);
    void ClearHitActors();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayHitSound(FVector Location, USoundBase* Sound);

    UFUNCTION(BlueprintPure, Category = "PT|Boss|Phase")
    int32 GetCurrentPhase() const;

    UFUNCTION(BlueprintPure, Category = "PT|Boss|Phase")
    float GetDamageMultiplierForPhase(int32 Phase) const;

    UAnimMontage* GetAttackMontageForPhase(int32 Phase) const;

    UPTBossPatternComponent* GetBossPatternComponent() const { return BossPatternComponent; }

    float GetCurrentMeleeDamage() const { return CurrentMeleeDamage; }
    const FPTHitInfo& GetCurrentMeleeHitInfo() const { return CurrentMeleeHitInfo; }

    void OnShieldDestroyed();

    UPROPERTY(BlueprintAssignable, Category = "PT|Boss|UI")
    FPTOnBossPhaseChanged OnPhaseChanged;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
    TObjectPtr<UPTBossPatternComponent> BossPatternComponent;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PT|Boss|Room")
    TObjectPtr<APTBossRoomCenter> RoomCenterActor;

protected:
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void OnDeath() override;
    virtual float StartAttack() override;
    virtual void StopAttack() override;
    virtual float GetAttackDamage() const override;
    virtual void ApplyHit(const FPTHitInfo& HitInfo) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    bool bHasShieldPhase = false;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    TSubclassOf<APTBossShield> ShieldClass;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    float ShieldMaxHP = 1500.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    float ShieldRadius = 500.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    float ShieldDuration = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    float ShieldPushForce = 1000.f;

    UPROPERTY()
    TObjectPtr<APTBossShield> ActiveShield;

    FTimerHandle ShieldTimerHandle;

    bool bShieldPhaseTriggered = false;

    UFUNCTION()
    void TryEnterShieldPhase(int32 NewPhase);

    void SpawnShield();
    
    UFUNCTION()
    void OnShieldTimerExpired();

    void FreezeForShieldPhase();
    void UnfreezeAfterShieldPhase();

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Animation")
    TObjectPtr<UAnimMontage> EnragedAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Animation")
    TObjectPtr<UAnimMontage> BerserkAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Phase")
    float Phase1DamageMultiplier = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Phase")
    float Phase2DamageMultiplier = 2.5f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Phase")
    float Phase1HPThreshold = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Phase")
    float Phase2HPThreshold = 0.1f;

    float CurrentMeleeDamage = 0.f;

    FPTHitInfo CurrentMeleeHitInfo;

    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;
};
