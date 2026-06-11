#include "Character/Monsters/AI/Tasks/PTBTTask_TurnToTarget.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UPTBTTask_TurnToTarget::UPTBTTask_TurnToTarget()
{
    NodeName = TEXT("Turn To Target");
}

EBTNodeResult::Type UPTBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!IsValid(AIC))
    {
        return EBTNodeResult::Failed;
    }

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(AIC->GetPawn());
    if (!IsValid(Monster))
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return EBTNodeResult::Failed;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
    if (!IsValid(Target))
    {
        return EBTNodeResult::Failed;
    }

    const FVector Direction = Target->GetActorLocation() - Monster->GetActorLocation();
    if (!Direction.IsNearlyZero())
    {
        const FVector ToTarget = Direction.GetSafeNormal();
        const FRotator LookAt = FRotationMatrix::MakeFromX(ToTarget).Rotator();
        Monster->SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
    }

    return EBTNodeResult::Succeeded;
}
