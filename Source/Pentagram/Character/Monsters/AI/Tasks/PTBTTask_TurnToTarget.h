#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PTBTTask_TurnToTarget.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTTask_TurnToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UPTBTTask_TurnToTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
