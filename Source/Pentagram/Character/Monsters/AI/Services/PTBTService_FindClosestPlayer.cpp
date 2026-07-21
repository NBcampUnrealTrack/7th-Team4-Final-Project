#include "Character/Monsters/AI/Services/PTBTService_FindClosestPlayer.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

UPTBTService_FindClosestPlayer::UPTBTService_FindClosestPlayer()
{
    NodeName        = TEXT("Find Closest Player");
    Interval        = 5.f;
    RandomDeviation = 0.5f;
}

void UPTBTService_FindClosestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIC       = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    APawn* OwnerPawn         = IsValid(AIC) ? AIC->GetPawn() : nullptr;
    UWorld* World            = IsValid(OwnerPawn) ? OwnerPawn->GetWorld() : nullptr;

    if (!IsValid(BB) || !IsValid(OwnerPawn) || !IsValid(World))
    {
        return;
    }

    const FVector MyLocation = OwnerPawn->GetActorLocation();

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    World->OverlapMultiByChannel(
        Overlaps,
        MyLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(SearchRadius),
        Params
    );

    APTPlayerCharacter* Closest = nullptr;
    float ClosestDistSq = FLT_MAX;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(Overlap.GetActor());
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
        BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, false);
        BB->ClearValue(PTMonsterBlackboardKeys::TargetActor);
        AIC->ClearFocus(EAIFocusPriority::Gameplay);
        return;
    }

    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsTargetDetected, true);

    AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
    if (CurrentTarget == Closest)
    {
        return;
    }

    BB->SetValueAsObject(PTMonsterBlackboardKeys::TargetActor, Closest);
    BB->ClearValue(PTMonsterBlackboardKeys::LastKnownLocation);
    AIC->SetFocus(Closest, EAIFocusPriority::Gameplay);
}
