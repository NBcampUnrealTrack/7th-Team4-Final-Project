#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PTAnimNotifyState_SkillBP.generated.h"

UCLASS()
class PENTAGRAM_API UPTAnimNotifyState_SkillBP : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

    // 스폰할 BP 액터
    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> SpawnBPClass;

    // 발동 지점을 TargetLocation(커서 지정 지점)으로 할지, 캐릭터 기준으로 할지
    UPROPERTY(EditAnywhere)
    bool bUseTargetLocation = true;

    // 캐릭터 기준일 때 오프셋 (앞/옆/위)
    UPROPERTY(EditAnywhere)
    FVector SpawnOffset = FVector::ZeroVector;

protected:
    // End에서 제거하려고 스폰된 액터 추적
    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> SpawnedActor;
};
