#include "Character/Monsters/PTMonsterAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTree.h"

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

    // TODO: APTMonsterCharacter 캐스팅 → RowKey로 DT_Character 조회
    //       SightConfig->SightRadius                    ← DT.SightRange
    //       SightConfig->LoseSightRadius                ← DT.ChaseRange
    //       SightConfig->PeripheralVisionAngleDegrees   ← DT.SightAngle / 2.f
    //       PerceptionComponent->RequestStimuliListenerUpdate()

    // TODO: BT 에셋 할당 후 RunBehaviorTree(BehaviorTree) 호출
}

void APTMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    // TODO: Day 5 Blackboard 설정 후 아래 키 갱신 구현
    //       IsTargetDetected ← Stimulus.WasSuccessfullySensed()
    //       TargetActor      ← Actor
}
