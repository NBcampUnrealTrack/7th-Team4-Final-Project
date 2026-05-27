#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PTBTService_MonsterSensor.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTService_MonsterSensor : public UBTService
{
    GENERATED_BODY()

public:
    UPTBTService_MonsterSensor();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
