#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PTAnimNotifyState_IceSkillProjectile.generated.h"

struct FProjectileData
{
    FVector CurrentPos  = FVector::ZeroVector;
    FVector Direction   = FVector::ForwardVector;
    bool    bExpired    = false;

     TSet<TWeakObjectPtr<AActor>> HitActors; // 투사체 개별 관리
};

UCLASS()
class PENTAGRAM_API UPTAnimNotifyState_IceSkillProjectile : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    int32 ProjectileCount = 10;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float SpreadAngleDegrees = 50.f;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float ProjectileSpeed = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float HitRadius = 80.f;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float MaxRange = 800.f;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

private:
    TArray<FProjectileData> Projectiles;

};
