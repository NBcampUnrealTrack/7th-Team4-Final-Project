#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_DodgeInvincibleStart.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_DodgeInvincibleStart : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
