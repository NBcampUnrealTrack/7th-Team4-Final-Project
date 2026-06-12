#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_AttackHit.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_AttackHit : public UAnimNotify
{
	GENERATED_BODY()

public:

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, Category = "VFX")
    TObjectPtr<class UNiagaraSystem> HitVFX;

    UPROPERTY(EditAnywhere, Category = "SFX")
    TObjectPtr<class USoundBase> HitSFX;

};
