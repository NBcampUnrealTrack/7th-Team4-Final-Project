#include "Character/Monsters/AI/Services/PTBTService_BossSensor.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UPTBTService_BossSensor::UPTBTService_BossSensor()
{
    NodeName        = TEXT("Boss Sensor");
    Interval        = 0.1f;
    RandomDeviation = 0.01f;
}

void UPTBTService_BossSensor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!IsValid(AIC))
    {
        return;
    }

    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(AIC->GetPawn());
    if (!IsValid(Boss))
    {
        return;
    }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
    if (!IsValid(Target))
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, false);
        return;
    }

    const float DistSq = FVector::DistSquared(Boss->GetActorLocation(), Target->GetActorLocation());
    const float AttackRangeSq = FMath::Square(Boss->GetAttackRange());
    const bool bInRange = (DistSq <= AttackRangeSq);

    if (BB->GetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange) != bInRange)
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, bInRange);
    }
}
