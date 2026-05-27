#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PTBTTask_Attack.generated.h"

struct FPTAttackTaskMemory
{
    FTimerHandle CooldownTimer;
};

UCLASS()
class PENTAGRAM_API UPTBTTask_Attack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPTBTTask_Attack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual uint16 GetInstanceMemorySize() const override;
};
