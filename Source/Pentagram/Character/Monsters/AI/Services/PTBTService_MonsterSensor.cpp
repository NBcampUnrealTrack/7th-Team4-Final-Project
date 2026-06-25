#include "Character/Monsters/AI/Services/PTBTService_MonsterSensor.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/PTBaseCharacter.h"

UPTBTService_MonsterSensor::UPTBTService_MonsterSensor()
{
    NodeName = TEXT("Monster Sensor");
}

void UPTBTService_MonsterSensor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!IsValid(AIC))
    {
        return;
    }

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(AIC->GetPawn());
    if (!IsValid(Monster))
    {
        return;
    }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return;
    }

    const FVector MonsterLocation = Monster->GetActorLocation();
    const float DistFromSpawnSq = FVector::DistSquared(MonsterLocation, Monster->GetSpawnLocation());
    const float ReturnThresholdSq = FMath::Square(Monster->GetPatrolRadius());

    BB->SetValueAsBool(PTMonsterBlackboardKeys::ShouldReturnToSpawn, DistFromSpawnSq > ReturnThresholdSq);

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));

    APTBaseCharacter* CurrentTarget = Cast<APTBaseCharacter>(Target);
    if (IsValid(CurrentTarget) && CurrentTarget->IsDead())
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
        BB->ClearValue(PTMonsterBlackboardKeys::TargetActor);
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, false);
        Target = nullptr;
    }

    if (!IsValid(Target))
    {
        if (UAIPerceptionComponent* PerceptionComp = AIC->GetPerceptionComponent())
        {
            TArray<AActor*> PerceivedActors;
            PerceptionComp->GetCurrentlyPerceivedActors(
                UAISense_Sight::StaticClass(), PerceivedActors);

            for (AActor* Actor : PerceivedActors)
            {
                APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(Actor);
                if (IsValid(Player) && Player->IsAlive())
                {
                    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, true);
                    BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, Player);
                    Target = Player;
                    break;
                }
            }
        }

        if (!IsValid(Target))
        {
            BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
            BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, false);
            return;
        }
    }

    const float DistToTargetSq = FVector::DistSquared(MonsterLocation, Target->GetActorLocation());

#if !UE_BUILD_SHIPPING
    const float DistToTarget = FMath::Sqrt(DistToTargetSq);
    UE_LOG(LogTemp, Log, TEXT("[Sensor] Dist: %.1f / AttackRange: %.1f / InRange: %s"),
        DistToTarget,
        Monster->GetAttackRange(),
        (DistToTarget <= Monster->GetAttackRange()) ? TEXT("TRUE") : TEXT("FALSE"));
#endif

    const float AttackRangeSq = FMath::Square(Monster->GetAttackRange());
    const float ChaseRangeSq  = FMath::Square(Monster->GetChaseRange());
    const float MaxChaseSq    = FMath::Square(Monster->GetMaxChaseDistance());

    const bool bOutOfChase = (DistToTargetSq > ChaseRangeSq || DistFromSpawnSq > MaxChaseSq);

    if (bOutOfChase)
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
        BB->ClearValue(PTMonsterBlackboardKeys::TargetActor);
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, false);
        return;
    }

    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, DistToTargetSq <= AttackRangeSq);
}
