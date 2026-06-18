#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/PTCombatTypes.h"
#include "PTBossProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;

UCLASS()
class PENTAGRAM_API APTBossProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	APTBossProjectile();

    void Launch(const FVector& Direction, float InDamage, float InSpeed, const FPTHitInfo& InHitInfo);

protected:
	virtual void BeginPlay() override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UPROPERTY(VisibleAnywhere, Category = "PT|Projectile")
    TObjectPtr<USphereComponent> CollisionComp;

    UPROPERTY(VisibleAnywhere, Category = "PT|Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(VisibleAnywhere, Category = "PT|Projectile")
    TObjectPtr<UNiagaraComponent> NiagaraComp;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Projectile")
    float LifeSpan = 5.f;

private:
    float Damage = 0.f;
    FPTHitInfo HitInfo;
};
