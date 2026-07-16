#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_SkillCastVFX.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_SkillCastVFX : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<class UNiagaraSystem> CastVFXOverride;
};
