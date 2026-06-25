#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_SuperArmorEnd.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_SuperArmorEnd : public UAnimNotify
{
	GENERATED_BODY()

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
};
