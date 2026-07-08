#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "Character/PTCombatTypes.h" // EHitReactionType, FPTHitInfo
#include "PTPlayerProjectileActor.generated.h"

class UNiagaraComponent;
class USoundBase;
class APTPlayerCharacter;
class USphereComponent;

UCLASS()
class PENTAGRAM_API APTPlayerProjectileActor : public AActor
{
    GENERATED_BODY()

public:
    APTPlayerProjectileActor();

    void InitProjectile(
        APTPlayerCharacter* InAttacker,
        const FVector& InDirection,
        float InSpeed,
        float InHitRadius,
        float InMaxRange,
        float InDamageMultiplier,
        bool bInPenetrate,
        bool bInApplyDamage,
        UNiagaraSystem* InHitEffect,
        UNiagaraSystem* InTrailEffect,
        USoundBase* InHitSound,
        float InKnockbackForce,
        float InKnockbackZForce,
        float InHitStopDuration,
        float InStaggerDuration,
        EHitReactionType InHitReactionType,
        bool bInDrawDebug);

    UFUNCTION()
    void OnProjectileOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    void PlayHitEffects(const FVector& Location);
private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> CollisionComp;

    // 이동을 그대로 따라가는 트레일 VFX
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UNiagaraComponent> TrailComponent;

    UPROPERTY()
    TWeakObjectPtr<APTPlayerCharacter> Attacker;

    FVector Direction = FVector::ForwardVector;
    float Speed = 1000.f;
    float HitRadius = 50.f;
    float MaxRange = 1500.f;
    float DamageMultiplier = 1.f;
    bool bPenetrate = false;
    bool bApplyDamage = false;
    bool bDrawDebug = false;

    float KnockbackForce = 300.f;
    float KnockbackZForce = 0.f;
    float HitStopDuration = 0.05f;
    float StaggerDuration = 0.3f;
    EHitReactionType HitReactionType = EHitReactionType::Light;

    UPROPERTY()
    TObjectPtr<USoundBase> HitSound;

    UPROPERTY()
    TObjectPtr<UNiagaraSystem> HitEffect;

    FVector StartLocation = FVector::ZeroVector;
    bool bExpired = false;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> HitActors;

    void ExpireProjectile();
};
