#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_SkillHit.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_SkillHit : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, Category = "VFX")
    TObjectPtr<class UNiagaraSystem> HitVFX;

    UPROPERTY(EditAnywhere, Category = "SFX")
    TObjectPtr<class USoundBase> HitSFX;
};
