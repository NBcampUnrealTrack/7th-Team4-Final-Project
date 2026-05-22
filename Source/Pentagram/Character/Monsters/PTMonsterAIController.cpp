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
    SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    Perception->ConfigureSense(*SightConfig);
    Perception->SetDominantSense(SightConfig->GetSenseImplementation());
    Perception->OnTargetPerceptionUpdated.AddDynamic(this, &APTMonsterAIController::OnTargetPerceptionUpdated);
}

void APTMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(InPawn);
    if (!Monster)
    {
        return;
    }

    if (SightConfig)
    {
        SightConfig->SightRadius = Monster->GetSightRange();
        SightConfig->LoseSightRadius = Monster->GetChaseRange();
        SightConfig->PeripheralVisionAngleDegrees = Monster->GetSightAngle() / 2.f;
    }

    UAIPerceptionComponent* PerceptionComp = GetPerceptionComponent();
    if (PerceptionComp)
    {
        PerceptionComp->RequestStimuliListenerUpdate();
    }

    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(PTMonsterBlackboardKeys::SpawnLocation, Monster->GetSpawnLocation());

        BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
    }
}

void APTMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    const bool bSensed = Stimulus.WasSuccessfullySensed();

    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, bSensed);
    BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, bSensed ? Actor : nullptr);
}
