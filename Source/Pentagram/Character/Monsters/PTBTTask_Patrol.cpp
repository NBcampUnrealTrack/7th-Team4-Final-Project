#include "Character/Monsters/PTBTTask_Patrol.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"

UPTBTTask_Patrol::UPTBTTask_Patrol()
{
    NodeName = TEXT("Patrol");
    BlackboardKey.SelectedKeyName = PTMonsterBlackboardKeys::PatrolLocation;
}

EBTNodeResult::Type UPTBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Failed;
    }

    UWorld* World = OwnerComp.GetWorld();
    if (!World)
    {
        return EBTNodeResult::Failed;
    }

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
    if (!NavSystem)
    {
        return EBTNodeResult::Failed;
    }

    const FVector SpawnLocation = BB->GetValueAsVector(PTMonsterBlackboardKeys::SpawnLocation);
    const float PatrolRadius = Monster->GetPatrolRadius();

    FNavLocation RandomLocation;
    if (!NavSystem->GetRandomReachablePointInRadius(SpawnLocation, PatrolRadius, RandomLocation))
    {
        return EBTNodeResult::Failed;
    }

    Monster->SetMonsterState(EMonsterState::Patrol);

    BB->SetValueAsVector(PTMonsterBlackboardKeys::PatrolLocation, RandomLocation.Location);

    return Super::ExecuteTask(OwnerComp, NodeMemory);
}
