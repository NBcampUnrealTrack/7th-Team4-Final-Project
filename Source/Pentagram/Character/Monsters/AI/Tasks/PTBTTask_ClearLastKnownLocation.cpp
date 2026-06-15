#include "Character/Monsters/AI/Tasks/PTBTTask_ClearLastKnownLocation.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"

UPTBTTask_ClearLastKnownLocation::UPTBTTask_ClearLastKnownLocation()
{
    NodeName = TEXT("Clear Last Known Location");
}

EBTNodeResult::Type UPTBTTask_ClearLastKnownLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return EBTNodeResult::Failed;
    }

    BB->ClearValue(PTMonsterBlackboardKeys::LastKnownLocation);

    return EBTNodeResult::Succeeded;
}
