#include "Character/Monsters/AI/Controllers/PTBaseAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "Character/Player/PTPlayerCharacter.h"

APTBaseAIController::APTBaseAIController()
{
    UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*Perception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    Perception->ConfigureSense(*SightConfig);
    Perception->SetDominantSense(SightConfig->GetSenseImplementation());
    Perception->OnTargetPerceptionUpdated.AddDynamic(this, &APTBaseAIController::OnTargetPerceptionUpdated);
}

void APTBaseAIController::UpdateSightConfig(float InSightRange, float InLoseSightRange, float InSightAngle)
{
    if (!IsValid(SightConfig))
    {
        return;
    }

    SightConfig->SightRadius = FMath::Max(0.f, InSightRange);
    SightConfig->LoseSightRadius = FMath::Max(0.f, InLoseSightRange);
    SightConfig->PeripheralVisionAngleDegrees = FMath::Clamp(InSightAngle / 2.f, 0.f, 180.f);

    if (UAIPerceptionComponent* P = GetPerceptionComponent())
    {
        P->ConfigureSense(*SightConfig);
        P->RequestStimuliListenerUpdate();
    }
}

void APTBaseAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!IsValid(BehaviorTree))
    {
        return;
    }

    if (!BehaviorTree->BlackboardAsset)
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = nullptr;
    if (!UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp))
    {
        return;
    }

    InitializeBlackboard(InPawn);
    PostPossessSetup(InPawn);
    RunBehaviorTree(BehaviorTree);
}

void APTBaseAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return;
    }

    if (!Cast<APTPlayerCharacter>(Actor))
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(Actor);
        if (IsValid(Player) && Player->IsDead())
        {
            if (BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor) == Player)
            {
                BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
                BB->ClearValue(PTMonsterBlackboardKeys::TargetActor);
                BB->SetValueAsBool(PTMonsterBlackboardKeys::IsInAttackRange, false);
                ClearFocus(EAIFocusPriority::Gameplay);
            }

            return;
        }

        AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
        if (!IsValid(CurrentTarget))
        {
            BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, true);
            BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, Actor);
            BB->ClearValue(PTMonsterBlackboardKeys::LastKnownLocation);
            SetFocus(Actor, EAIFocusPriority::Gameplay);
            return;
        }

        if (TargetSwitchDelay <= 0.f)
        {
            SwitchToClosestPerceivedPlayer();
        }
        else if (!GetWorldTimerManager().IsTimerActive(TargetSwitchTimerHandle))
        {
            GetWorldTimerManager().SetTimer(
                TargetSwitchTimerHandle,
                this, &APTBaseAIController::SwitchToClosestPerceivedPlayer,
                TargetSwitchDelay, false);
        }
    }
    else
    {
        if (!IsValid(Actor))
        {
            return;
        }

        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
        BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, nullptr);
        BB->SetValueAsVector(PTMonsterBlackboardKeys::LastKnownLocation, Actor->GetActorLocation());
        ClearFocus(EAIFocusPriority::Gameplay);

        GetWorldTimerManager().ClearTimer(TargetSwitchTimerHandle);
    }
}

void APTBaseAIController::SwitchToClosestPerceivedPlayer()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    UAIPerceptionComponent* Perception = GetPerceptionComponent();
    APawn* OwnerPawn = GetPawn();
    if (!IsValid(BB) || !IsValid(Perception) || !IsValid(OwnerPawn))
    {
        return;
    }

    TArray<AActor*> PerceivedActors;
    Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    APTPlayerCharacter* Closest = nullptr;
    float ClosestDistSq = FLT_MAX;
    const FVector MyLocation = OwnerPawn->GetActorLocation();

    for (AActor* PerceivedActor : PerceivedActors)
    {
        APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(PerceivedActor);
        if (!IsValid(Player) || Player->IsDead())
        {
            continue;
        }

        const float DistSq = FVector::DistSquared(MyLocation, Player->GetActorLocation());
        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            Closest = Player;
        }
    }

    if (!IsValid(Closest))
    {
        return;
    }

    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, true);
    BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, Closest);
    BB->ClearValue(PTMonsterBlackboardKeys::LastKnownLocation);
    SetFocus(Closest, EAIFocusPriority::Gameplay);
}
