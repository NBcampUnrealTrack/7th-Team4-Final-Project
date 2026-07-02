#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PTBTService_BossSensor.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTService_BossSensor : public UBTService
{
	GENERATED_BODY()

public:
    UPTBTService_BossSensor();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
