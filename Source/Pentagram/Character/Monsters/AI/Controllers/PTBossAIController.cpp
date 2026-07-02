#include "Character/Monsters/AI/Controllers/PTBossAIController.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"

APTBossAIController::APTBossAIController()
{
}

void APTBossAIController::InitializeBlackboard(APawn* InPawn)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return;
    }

    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(InPawn);
    if (!IsValid(Boss))
    {
        return;
    }

    BB->SetValueAsVector(PTMonsterBlackboardKeys::SpawnLocation, Boss->GetActorLocation());

    CachedPhase = Boss->GetCurrentPhase();
    BB->SetValueAsInt(PTMonsterBlackboardKeys::BossPhase, CachedPhase);
    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsPatternActive, false);
    BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsRanged, Boss->IsRangedMonster());
    BB->SetValueAsFloat(PTMonsterBlackboardKeys::OptimalRange, Boss->GetOptimalRange());
}

void APTBossAIController::PostPossessSetup(APawn* InPawn)
{
    if (APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(InPawn))
    {
        Boss->OnHPChanged.AddDynamic(this, &APTBossAIController::OnBossHPChanged);
    }
}

void APTBossAIController::OnBossHPChanged(float CurrentHP, float MaxHP)
{
    APTBossMonsterCharacter* Boss = Cast<APTBossMonsterCharacter>(GetPawn());
    if (!IsValid(Boss))
    {
        return;
    }

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return;
    }

    const int32 NewPhase = Boss->GetCurrentPhase();

    if (NewPhase != CachedPhase)
    {
        CachedPhase = NewPhase;
        BB->SetValueAsInt(PTMonsterBlackboardKeys::BossPhase, NewPhase);
        Boss->OnPhaseChanged.Broadcast(NewPhase);
    }
}
