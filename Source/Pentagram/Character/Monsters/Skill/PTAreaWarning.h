#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTAreaWarning.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class UStaticMeshComponent;
class UAudioComponent;
class USoundBase;

UCLASS()
class PENTAGRAM_API APTAreaWarning : public AActor
{
	GENERATED_BODY()
	
public:	
	APTAreaWarning();

    void Launch(const FVector& GroundLocation, float InFallDuration,
        UNiagaraSystem* InFallEffect, UNiagaraSystem* InImpactEffect,
        float StartHeight = 2000.f, float InMaxRedius = 300.f, bool bInGroundMode = false);

protected:
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void OnLanded();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UNiagaraComponent> FallEffectComp;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> WarningMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BorderMesh;

    UPROPERTY(VisibleAnywhere, Category = "PT|Sound")
    TObjectPtr<UAudioComponent> BuildupAudioComp;

    UPROPERTY(EditAnywhere, Category = "PT|Sound")
    TSoftObjectPtr<USoundBase> BuildupSound;

    UPROPERTY(EditAnywhere, Category = "PT|Sound")
    TSoftObjectPtr<USoundBase> ExplosionSound;

    FVector StartLocation;
    FVector TargetLocation;
    float FallDuration      = 1.f;
    float ElapsedTime       = 0.f;
    bool bLaunched          = false;
    bool bLanded            = false;
    float MaxRadius         = 300.f;
    float CachedStartHeight = 2000.f;
    bool bGroundMode        = false;

    TObjectPtr<UNiagaraSystem> PendingImpactEffect;
};
