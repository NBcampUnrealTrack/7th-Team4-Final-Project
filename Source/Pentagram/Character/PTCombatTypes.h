#pragma once

#include "CoreMinimal.h"
#include "PTCombatTypes.generated.h"

UENUM(BlueprintType)
enum class EHitReactionType : uint8
{
    Light,
    Heavy
};

USTRUCT(BlueprintType)
struct FPTHitInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float KnockbackForce = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float KnockbackZForce = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HitStopDuration = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaggerDuration = 0.3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHitReactionType HitReactionType = EHitReactionType::Light;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HitDirection = FVector::ZeroVector;

    UPROPERTY()
    AActor* Attacker = nullptr;
};
