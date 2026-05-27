#include "Character/Monsters/PTBTTask_MoveToTarget.h"
#include "Character/Monsters/PTMonsterBlackboardKeys.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "AIController.h"

UPTBTTask_MoveToTarget::UPTBTTask_MoveToTarget()
{
    NodeName = TEXT("Move To Target");
    BlackboardKey.SelectedKeyName = PTMonsterBlackboardKeys::TargetActor;
}

EBTNodeResult::Type UPTBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC)
    {
        return EBTNodeResult::Failed;
    }

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(AIC->GetPawn());
    if (!Monster)
    {
        return EBTNodeResult::Failed;
    }

    Monster->SetMonsterState(EMonsterState::Chase);

    return Super::ExecuteTask(OwnerComp, NodeMemory);
}
