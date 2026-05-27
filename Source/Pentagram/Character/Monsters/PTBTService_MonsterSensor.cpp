#include "Character/Monsters/PTBTService_MonsterSensor.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UPTBTService_MonsterSensor::UPTBTService_MonsterSensor()
{
    NodeName = TEXT("Monster Sensor");
}

void UPTBTService_MonsterSensor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC)
    {
        return;
    }

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(AIC->GetPawn());
    if (!Monster)
    {
        return;
    }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
    if (!IsValid(Target))
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
        BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, nullptr);
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, false);
        return;
    }

    const float DistToTarget  = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
    const float DistFromSpawn = FVector::Dist(Monster->GetActorLocation(), Monster->GetSpawnLocation());

    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, DistToTarget <= Monster->GetAttackRange());

    const bool bOutOfChase = DistToTarget > Monster->GetChaseRange() || DistFromSpawn > Monster->GetMaxChaseDistance();

    if (bOutOfChase)
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
        BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, nullptr);
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, false);
    }
}
