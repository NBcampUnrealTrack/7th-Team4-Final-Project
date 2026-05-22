#include "Character/Monsters/BTTask_Patrol.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NavigationSystem.h"                
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTMonsterBlackboardKeys.h"
#include "Character/Monsters/PTMonsterState.h"

UBTTask_Patrol::UBTTask_Patrol()
{
    NodeName = TEXT("Patrol");
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{


    return EBTNodeResult::Type();
}

EBTNodeResult::Type UBTTask_Patrol::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::Type();
}
