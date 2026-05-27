#include "Character/Monsters/PTBTTask_Attack.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTMonsterBlackboardKeys.h"
#include "Character/Monsters/PTMonsterState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

UPTBTTask_Attack::UPTBTTask_Attack()
{
    NodeName = TEXT("Attack");
}

EBTNodeResult::Type UPTBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FPTAttackTaskMemory* Memory = CastInstanceNodeMemory<FPTAttackTaskMemory>(NodeMemory);

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

    BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, false);
    Monster->SetMonsterState(EMonsterState::Attack);
    Monster->PerformAttack();

    const float AttackDuration = Monster->GetAttackSpeed() > 0.f ? (1.f / Monster->GetAttackSpeed()) : 1.f;

    TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp(&OwnerComp);
    TWeakObjectPtr<UBlackboardComponent> WeakBlackboard(BB);

    OwnerComp.GetWorld()->GetTimerManager().SetTimer(
        Memory->CooldownTimer,
        FTimerDelegate::CreateWeakLambda(
            this,
            [this, WeakOwnerComp, WeakBlackboard]()
            {
                if (WeakBlackboard.IsValid())
                {
                    WeakBlackboard->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
                }
                if (WeakOwnerComp.IsValid())
                {
                    FinishLatentTask(*WeakOwnerComp.Get(), EBTNodeResult::Succeeded);
                }
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
