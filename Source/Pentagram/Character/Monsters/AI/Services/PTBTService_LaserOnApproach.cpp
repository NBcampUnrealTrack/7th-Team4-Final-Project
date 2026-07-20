#include "Character/Monsters/AI/Services/PTBTService_LaserOnApproach.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

struct FPTLaserServiceMemory
{
    FTimerHandle CanAttackRestoreTimer;
};

UPTBTService_LaserOnApproach::UPTBTService_LaserOnApproach()
{
    NodeName = TEXT("Laser On Approach");

    Interval = 5.f;
    RandomDeviation = 0.5f;
}

uint16 UPTBTService_LaserOnApproach::GetInstanceMemorySize() const
{

    return sizeof(FPTLaserServiceMemory);
}

void UPTBTService_LaserOnApproach::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    TickNode(OwnerComp, NodeMemory, 0.f);
}

void UPTBTService_LaserOnApproach::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!IsValid(AIC))
    {
        return;
    }

    UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return;
    }

    if (!BB->GetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected))
    {
        return;
    }

    if (!BB->GetValueAsBool(PTMonsterBlackboardKeys::CanAttack))
    {
        return;
    }

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(AIC->GetPawn());
    AActor* Target = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
    if (!IsValid(Monster) || !IsValid(Target))
    {
        return;
    }

    const float Dist = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
    if (Dist <= LaserFireMinDistance)
    {
        return;
    }

    FPTLaserServiceMemory* Memory = CastInstanceNodeMemory<FPTLaserServiceMemory>(NodeMemory);
    if (!Memory)
    {
        return;
    }

    BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, false);
    const float Duration = Monster->StartAttack();

    TWeakObjectPtr<UBlackboardComponent> WeakBB(BB);
    TWeakObjectPtr<APTMonsterCharacter> WeakMonster(Monster);

    OwnerComp.GetWorld()->GetTimerManager().SetTimer(
        Memory->CanAttackRestoreTimer,
        FTimerDelegate::CreateLambda([WeakBB, WeakMonster]()
            {
                APTMonsterCharacter* MonsterPtr = WeakMonster.Get();
                UBlackboardComponent* BBPtr = WeakBB.Get();

                if (!IsValid(MonsterPtr) || !IsValid(BBPtr))
                {
                    return;
                }

                if (MonsterPtr->IsDead())
                {
                    return;
                }

                if (BBPtr->GetValueAsBool(PTMonsterBlackboardKeys::IsPatternActive))
                {
                    return;
                }

                BBPtr->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
            }),
        FMath::Max(Duration, 1.f),
        false
    );
}

void UPTBTService_LaserOnApproach::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnCeaseRelevant(OwnerComp, NodeMemory);

    FPTLaserServiceMemory* Memory = CastInstanceNodeMemory<FPTLaserServiceMemory>(NodeMemory);
    if (!Memory)
    {
        return;
    }

    UWorld* World = OwnerComp.GetWorld();
    if (!World)
    {
        return;
    }

    FTimerManager& TM = World->GetTimerManager();
    if (TM.IsTimerActive(Memory->CanAttackRestoreTimer))
    {
        if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
        {
            BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
        }

        TM.ClearTimer(Memory->CanAttackRestoreTimer);
    }
}
