#include "Character/Monsters/AI/Services/PTBTService_MonsterSensor.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
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

    const FVector MonsterLocation = Monster->GetActorLocation();
    const float DistToTargetSq  = FVector::DistSquared(MonsterLocation, Target->GetActorLocation());
    const float DistFromSpawnSq = FVector::DistSquared(MonsterLocation, Monster->GetSpawnLocation());

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("[Sensor] Dist: %.1f / AttackRange: %.1f / InRange: %s"),
        FMath::Sqrt(DistToTargetSq),
        Monster->GetAttackRange(),
        (FMath::Sqrt(DistToTargetSq) <= Monster->GetAttackRange()) ? TEXT("TRUE") : TEXT("false"));
#endif

    const float AttackRangeSq = FMath::Square(Monster->GetAttackRange());
    const float ChaseRangeSq  = FMath::Square(Monster->GetChaseRange());
    const float MaxChaseSq    = FMath::Square(Monster->GetMaxChaseDistance());

    const bool bOutOfChase = (DistToTargetSq > ChaseRangeSq || DistFromSpawnSq > MaxChaseSq);

    if (bOutOfChase)
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
        BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, nullptr);
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, false);
        return;
    }

    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, DistToTargetSq <= AttackRangeSq);
}
