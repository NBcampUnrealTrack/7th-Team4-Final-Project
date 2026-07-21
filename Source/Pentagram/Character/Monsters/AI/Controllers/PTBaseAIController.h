#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "PTBaseAIController.generated.h"

class UAISenseConfig_Sight;
class UBehaviorTree;

UCLASS(Abstract)
class PENTAGRAM_API APTBaseAIController : public AAIController
{
	GENERATED_BODY()

public:
    APTBaseAIController();

    void UpdateSightConfig(float InSightRange, float InLoseSightRange, float InSightAngle);

protected:
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION()
    virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    virtual void InitializeBlackboard(APawn* InPawn) {}

    virtual void PostPossessSetup(APawn* InPawn) {}

    UPROPERTY(EditDefaultsOnly, Category = "PT|AI|BehaviorTree")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "PT|AI|Perception")
    float TargetSwitchDelay = 0.f;

private:
    void SwitchToClosestPerceivedPlayer();

    FTimerHandle TargetSwitchTimerHandle;

    UPROPERTY(VisibleAnywhere, Category = "PT|AI|Perception")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;
};
