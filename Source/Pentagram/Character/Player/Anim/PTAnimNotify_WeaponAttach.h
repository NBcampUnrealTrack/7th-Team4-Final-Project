#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_WeaponAttach.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_WeaponAttach : public UAnimNotify
{
    GENERATED_BODY()

    public:

    UPROPERTY(EditAnywhere, Category = "Weapon")
    bool bAttachToHand = true;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override;
};
