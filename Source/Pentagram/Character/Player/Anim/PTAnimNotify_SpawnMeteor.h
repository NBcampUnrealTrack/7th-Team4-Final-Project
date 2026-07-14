#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PTAnimNotify_SpawnMeteor.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotify_SpawnMeteor : public UAnimNotify
{
    GENERATED_BODY()

public:
    void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere)
    FName SkillRowName;

    UPROPERTY(EditAnywhere)
    TSubclassOf<class APTMeteorActor> MeteorClass; // 여기에 만든 BP 지정

    UPROPERTY(EditAnywhere)
    float FallHeight = 2000.f;

    UPROPERTY(EditAnywhere)
    float FallSpeed  = 3000.f;

    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UNiagaraSystem> ImpactVFXOverride;

    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<USoundBase> ImpactSoundOverride;

    UPROPERTY(EditAnywhere)
    bool bDrawDebug = false;
};
