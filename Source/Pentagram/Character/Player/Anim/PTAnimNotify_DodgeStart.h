#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_DodgeStart.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_DodgeStart : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, Category = "Dodge")
    float DodgeDistance = 500.f;
};
