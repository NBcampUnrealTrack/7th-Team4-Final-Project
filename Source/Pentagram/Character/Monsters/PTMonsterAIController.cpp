#include "Character/Monsters/PTMonsterAIController.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/PTMonsterBlackboardKeys.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

APTMonsterAIController::APTMonsterAIController()
{
    UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*Perception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    Perception->ConfigureSense(*SightConfig);
    Perception->SetDominantSense(SightConfig->GetSenseImplementation());
    Perception->OnTargetPerceptionUpdated.AddDynamic(this, &APTMonsterAIController::OnTargetPerceptionUpdated);
}

void APTMonsterAIController::UpdateSightConfig(float InSightRange, float InLoseSightRange, float InSightAngle)
{
    if (!SightConfig)
    {
        return;
    }

    SightConfig->SightRadius = InSightRange;
    SightConfig->LoseSightRadius = InLoseSightRange;
    SightConfig->PeripheralVisionAngleDegrees = InSightAngle / 2.f;

    if (UAIPerceptionComponent* PerceptionComp = GetPerceptionComponent())
    {
        PerceptionComp->ConfigureSense(*SightConfig);
        PerceptionComp->RequestStimuliListenerUpdate();
    }
}

void APTMonsterAIController::UpdateMonsterBlackboard(APTMonsterCharacter* Monster)
{
    if (!Monster)
    {
        return;
    }

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    BB->SetValueAsVector(PTMonsterBlackboardKeys::SpawnLocation, Monster->GetSpawnLocation());
    BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
}

void APTMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(InPawn);
    if (!Monster)
    {
        return;
    }

    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void APTMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, true);
        BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, Actor);
    }
}
