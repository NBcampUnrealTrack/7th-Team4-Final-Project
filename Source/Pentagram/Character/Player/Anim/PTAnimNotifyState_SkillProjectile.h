#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Character/PTCombatTypes.h"
#include "PTAnimNotifyState_SkillProjectile.generated.h"

USTRUCT()
struct FProjectileRuntimeData
{
    GENERATED_BODY()

    FVector CurrentPos = FVector::ZeroVector;
    FVector Direction  = FVector::ForwardVector;
    bool bExpired = false;

    // 이 투사체를 따라다니며 이동하는 나이아가라 컴포넌트
    UPROPERTY()
    TObjectPtr<UNiagaraComponent> TrailComponent = nullptr;

    TArray<TWeakObjectPtr<AActor>> HitActors;
};

UCLASS()
class PENTAGRAM_API UPTAnimNotifyState_SkillProjectile : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    // 이 노티파이가 참조할 DT_Skillrow의 로우, CurrentSkillID와 무관하게 항상 이 값으로 SkillData를 조회
    UPROPERTY(EditAnywhere, Category = "Projectile")
    FName SkillRowName = NAME_None;

    // 투사체 개수/퍼짐/속도/판정은 몽타주별로 다를 수 있어 노티파이 자체 파라미터로 유지
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

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

    // 비워두면 DT_Skillrow의 ProjectileEffect를 그대로 사용 (값을 채우면 DT 값 대신 이 이펙트를 사용)
    UPROPERTY(EditAnywhere, Category = "Projectile")
    TSoftObjectPtr<UNiagaraSystem> ProjectileEffectOverride;

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
    UPROPERTY()
    TArray<FProjectileRuntimeData> Projectiles;

    void ExpireProjectile(FProjectileRuntimeData& Proj);
};

