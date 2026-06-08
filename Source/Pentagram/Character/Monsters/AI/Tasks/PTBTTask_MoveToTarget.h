#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "PTBTTask_MoveToTarget.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTTask_MoveToTarget : public UBTTask_MoveTo
{
    GENERATED_BODY()

public:
    UPTBTTask_MoveToTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
