#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PTAnimNotifyState_PlayRate.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotifyState_PlayRate : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayRate")
    float TargetPlayRate = 0.4f;

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override
    {
        return FString::Printf(TEXT("PlayRate x%.2f"), TargetPlayRate);
    }
};
