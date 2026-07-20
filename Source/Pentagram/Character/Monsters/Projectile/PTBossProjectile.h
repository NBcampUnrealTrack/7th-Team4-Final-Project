#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/PTCombatTypes.h"
#include "PTBossProjectile.generated.h"


class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UAudioComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class PENTAGRAM_API APTBossProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	APTBossProjectile();

    void Launch(const FVector& Direction, float InDamage, float InSpeed, const FPTHitInfo& InHitInfo, float InHomingStrength = 0.f, AActor* InHomingTarget = nullptr);
    void IgnoreActor(AActor* ActorToIgnore);

protected:
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayImpactSound(FVector Location);

    UPROPERTY(VisibleAnywhere, Category = "PT|Projectile")
    TObjectPtr<USphereComponent> CollisionComp;

    UPROPERTY(VisibleAnywhere, Category = "PT|Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(VisibleAnywhere, Category = "PT|Projectile")
    TObjectPtr<UNiagaraComponent> NiagaraComp;

    UPROPERTY(VisibleAnywhere, Category = "PT|Projectile")
    TObjectPtr<UStaticMeshComponent> MeshComp;

    UPROPERTY(VisibleAnywhere, Category = "PT|Sound")
    TObjectPtr<UAudioComponent> FlightAudioComp;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Projectile")
    float LifeSpan = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Projectile")
    float CollisionRadius = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Sound")
    TSoftObjectPtr<USoundBase> FlightSound;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Sound")
    TSoftObjectPtr<USoundBase> ImpactSound;

private:
    float Damage = 0.f;
    FPTHitInfo HitInfo;

    float HomingStrength = 0.f;
    TWeakObjectPtr<AActor> HomingTarget;
};
