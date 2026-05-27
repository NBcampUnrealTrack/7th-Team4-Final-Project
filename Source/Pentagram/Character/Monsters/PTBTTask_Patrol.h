#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "PTBTTask_Patrol.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTTask_Patrol : public UBTTask_MoveTo
{
    GENERATED_BODY()

public:
    UPTBTTask_Patrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
