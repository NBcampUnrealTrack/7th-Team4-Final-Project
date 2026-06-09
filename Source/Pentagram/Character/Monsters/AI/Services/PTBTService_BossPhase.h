#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PTBTService_BossPhase.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTService_BossPhase : public UBTService
{
    GENERATED_BODY()

public:
    UPTBTService_BossPhase();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
