#include "Character/Monsters/AI/Tasks/PTBTTask_Attack.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "Character/Monsters/PTMonsterState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

UPTBTTask_Attack::UPTBTTask_Attack()
{
    NodeName = TEXT("Attack");
}

EBTNodeResult::Type UPTBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FPTAttackTaskMemory* Memory = CastInstanceNodeMemory<FPTAttackTaskMemory>(NodeMemory);

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

    UWorld* World = OwnerComp.GetWorld();
    if (!IsValid(World))
    {
        return EBTNodeResult::Failed;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
    if (IsValid(Target))
    {
        const FVector Direction = Target->GetActorLocation() - Monster->GetActorLocation();
        if (!Direction.IsNearlyZero())
        {
            const FVector ToTarget = Direction.GetSafeNormal();
            const FRotator LookAt = FRotationMatrix::MakeFromX(ToTarget).Rotator();
            Monster->SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
        }
    }

    BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, false);
    Monster->SetMonsterState(EMonsterState::Attack);

    const float AttackDuration = Monster->StartAttack();

    if (AttackDuration <= 0.f)
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
        Monster->SetMonsterState(EMonsterState::Idle);
        return EBTNodeResult::Failed;
    }

    TWeakObjectPtr<UPTBTTask_Attack> WeakThis(this);
    TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp(&OwnerComp);
    TWeakObjectPtr<UBlackboardComponent> WeakBlackboard(BB);

    World->GetTimerManager().SetTimer(
        Memory->CooldownTimer,
        FTimerDelegate::CreateWeakLambda(
            this,
            [WeakThis, WeakOwnerComp, WeakBlackboard]()
            {
                if (!WeakThis.IsValid() || !WeakOwnerComp.IsValid() || !WeakBlackboard.IsValid())
                {
                    return;
                }

                UBehaviorTreeComponent* OwnerCompPtr = WeakOwnerComp.Get();
                UBlackboardComponent* BlackboardPtr = WeakBlackboard.Get();

                if (!OwnerCompPtr || !BlackboardPtr)
                {
                    return;
                }

                BlackboardPtr->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
                WeakThis->FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
            }),
        AttackDuration, false);

    return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UPTBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FPTAttackTaskMemory* Memory = CastInstanceNodeMemory<FPTAttackTaskMemory>(NodeMemory);

    if (UWorld* World = OwnerComp.GetWorld())
    {
        World->GetTimerManager().ClearTimer(Memory->CooldownTimer);
    }

    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        if (APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(AIC->GetPawn()))
        {
            Monster->StopAttack();
            Monster->SetMonsterState(EMonsterState::Idle);
        }
    }

    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
    }

    return EBTNodeResult::Aborted;
}

uint16 UPTBTTask_Attack::GetInstanceMemorySize() const
{
    return sizeof(FPTAttackTaskMemory);
}
