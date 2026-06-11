#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PTBTTask_ClearLastKnownLocation.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTTask_ClearLastKnownLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UPTBTTask_ClearLastKnownLocation();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
