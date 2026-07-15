#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_PlaySound.generated.h"

class USoundBase;

UCLASS()
class PENTAGRAM_API UPTAnimNotify_PlaySound : public UAnimNotify
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "PT|Sound")
    TSoftObjectPtr<USoundBase> Sound;

    UPROPERTY(EditAnywhere, Category = "PT|Sound")
    FName SocketName = NAME_None;

    UPROPERTY(EditAnywhere, Category = "PT|Sound")
    float VolumeMultiplier = 1.f;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
