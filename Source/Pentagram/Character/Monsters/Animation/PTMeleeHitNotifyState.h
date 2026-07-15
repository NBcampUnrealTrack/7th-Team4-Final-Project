#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PTMeleeHitNotifyState.generated.h"

class APTBossMonsterCharacter;
class SoundBase;

UCLASS()
class PENTAGRAM_API UPTMeleeHitNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "PT|Melee")
    FName HitSocketName = TEXT("fist_r");

    UPROPERTY(EditAnywhere, Category = "PT|Melee")
    float HitRadius = 50.f;

    UPROPERTY(EditAnywhere, Category = "PT|Melee")
    float StartCheckRadius = 150.f;

    UPROPERTY(EditAnywhere, Category = "PT|Melee")
    float HomingStrength = 5.f;

    UPROPERTY(EditAnywhere, Category = "PT|Sound")
    TSoftObjectPtr<USoundBase> HitSound;

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
    TMap<USkeletalMeshComponent*, FVector> PrevSocketLocations;

    APTBossMonsterCharacter* GetBoss(USkeletalMeshComponent* MeshComp) const;
    AActor* GetTarget(APTBossMonsterCharacter* Boss) const;
   
};
