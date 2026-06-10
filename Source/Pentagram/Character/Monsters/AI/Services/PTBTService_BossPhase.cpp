#include "Character/Monsters/AI/Services/PTBTService_BossPhase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"

UPTBTService_BossPhase::UPTBTService_BossPhase()
{
    NodeName = TEXT("Boss Phase");

    Interval = 0.2f;
    RandomDeviation = 0.05f;
}

void UPTBTService_BossPhase::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

    const int32 NewPhase     = Boss->GetCurrentPhase();
    const int32 CurrentPhase = BB->GetValueAsInt(PTMonsterBlackboardKeys::BossPhase);

    if (NewPhase > CurrentPhase)
    {
        BB->SetValueAsInt(PTMonsterBlackboardKeys::BossPhase, NewPhase);
        Boss->OnPhaseChanged.Broadcast(NewPhase);
    }
}
