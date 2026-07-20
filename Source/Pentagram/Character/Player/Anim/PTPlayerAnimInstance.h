#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PTPlayerAnimInstance.generated.h"

UCLASS()
class PENTAGRAM_API UPTPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

protected:
    virtual void NativeInitializeAnimation() override;
};
