#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_AttackSFX.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_AttackSFX : public UAnimNotify
{
    GENERATED_BODY()

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, Category = "SFX")
    TObjectPtr<USoundBase> SoundAsset;
};
