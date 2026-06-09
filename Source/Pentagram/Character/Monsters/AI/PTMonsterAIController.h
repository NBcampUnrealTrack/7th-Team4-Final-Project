#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "PTMonsterAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;
class APTMonsterCharacter;

UCLASS()
class PENTAGRAM_API APTMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
    APTMonsterAIController();

    void UpdateSightConfig(float InSightRange, float InLoseSightRange, float InSightAngle);
    void UpdateMonsterBlackboard(APTMonsterCharacter* Monster);

protected:
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
    UPROPERTY(VisibleAnywhere, Category = "PT|AI|Perception")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(EditDefaultsOnly, Category = "PT|AI|BehaviorTree")
    TObjectPtr<UBehaviorTree> BehaviorTree;
};
