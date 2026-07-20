#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/PTCombatTypes.h"

#include "PTMeteorActor.generated.h"

class APTPlayerCharacter;
class UNiagaraSystem;

UCLASS()
class PENTAGRAM_API APTMeteorActor : public AActor
{
    GENERATED_BODY()

public:
    APTMeteorActor();

    void InitMeteor(
        APTPlayerCharacter* InAttacker,
        const FVector& InTargetGround,   // 커서로 지정한 착지 지점
        float InFallHeight,              // 이 높이만큼 위에서 시작
        float InFallSpeed,
        float InDamageRadius,
        float InDamageMultiplier,
        bool  bInApplyDamage,            // 서버만 true
        UNiagaraSystem* InImpactVFX,
        USoundBase* InImpactSound,
        const FPTHitInfo& InHitTemplate);

protected:
    virtual void Tick(float DeltaTime) override;
    void OnImpact();

    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;
    // 돌 메시 / 낙하 트레일 VFX는 BP에서 Root에 붙임

    TWeakObjectPtr<APTPlayerCharacter> Attacker;
    FVector TargetGround = FVector::ZeroVector;
    float FallSpeed = 3000.f;
    float DamageRadius = 300.f;
    float DamageMultiplier = 1.f;
    bool  bApplyDamage = false;
    UPROPERTY() UNiagaraSystem* ImpactVFX = nullptr;
    UPROPERTY() USoundBase* ImpactSound = nullptr;
    FPTHitInfo HitTemplate;
    bool  bImpacted = false;
};
