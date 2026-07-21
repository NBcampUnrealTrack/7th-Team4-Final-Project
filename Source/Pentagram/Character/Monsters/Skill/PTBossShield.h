#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "PTBossShield.generated.h"

class APTBossMonsterCharacter;
class UNiagaraSystem;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USoundBase;

UCLASS()
class PENTAGRAM_API APTBossShield : public APTBaseCharacter
{
	GENERATED_BODY()

public:
    APTBossShield();

    void InitShield(APTBossMonsterCharacter* InOwningBoss, float InMaxHP, float InRadius, float InPushForce);

    virtual float ApplyDamageWithHit(float DamageAmount, AActor* Attacker, const FPTHitInfo& HitInfo) override;

    float GetCurrentShieldHP() const { return CurrentHP; }

    void SuppressDestroyNotify() { bSuppressDestroyNotify = true; }

protected:
    virtual void ApplyHit(const FPTHitInfo& HitInfo) override;
    virtual void OnDeath() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    TSoftObjectPtr<UMaterialInterface> ShieldMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    float ShieldMeshBaseRadius = 50.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    TSoftObjectPtr<UNiagaraSystem> HitImpactEffect;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Shield")
    TSoftObjectPtr<UNiagaraSystem> BreakEffect;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Sound")
    TSoftObjectPtr<USoundBase> HitSound;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Sound")
    TSoftObjectPtr<USoundBase> BreakSound;

private:
    void PushActorOut(AActor* TargetActor);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastInitShieldVisual(float InRadius);

    UPROPERTY(VisibleAnywhere, Category = "PT|Boss|Shield")
    TObjectPtr<UStaticMeshComponent> ShieldMeshComp;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> ShieldMID;

    TWeakObjectPtr<APTBossMonsterCharacter> OwningBoss;

    float ShieldRadius = 500.f;
    float PushForce = 1500.f;

    bool bSuppressDestroyNotify = false;
};
