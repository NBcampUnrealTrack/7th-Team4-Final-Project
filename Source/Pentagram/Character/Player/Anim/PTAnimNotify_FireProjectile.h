#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "NiagaraSystem.h"
#include "PTAnimNotify_FireProjectile.generated.h"

UCLASS(meta = (DisplayName = "PT Fire Projectile"))
class PENTAGRAM_API UPTAnimNotify_FireProjectile : public UAnimNotify
{
    GENERATED_BODY()

public:
    // 참조할 DT_Skillrow의 로우 이름
    UPROPERTY(EditAnywhere, Category = "Projectile")
    FName SkillRowName = NAME_None;

    UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = "1"))
    int32 ProjectileCount = 1;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float SpreadAngleDegrees = 0.f;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float ProjectileSpeed = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float HitRadius = 50.f;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float MaxRange = 1500.f;

    // 비워두면 DT의 ProjectileEffect 사용, 채워두면 이 노티파이 값을 우선 사용
    UPROPERTY(EditAnywhere, Category = "Projectile")
    TSoftObjectPtr<UNiagaraSystem> ProjectileEffectOverride;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    TSoftObjectPtr<USoundBase> HitSoundOverride;

    // 다른 액터와 충돌 시 재생할 히트 VFX
    UPROPERTY(EditAnywhere, Category = "Projectile")
    TSoftObjectPtr<UNiagaraSystem> HitEffect;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

    UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay)
    float MinRefireInterval = 0.05f;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

#if WITH_EDITOR
    virtual FString GetNotifyName_Implementation() const override { return TEXT("PT Fire Projectile"); }
#endif

    TMap<TWeakObjectPtr<AActor>, float> LastFireTimeByOwner;
};
