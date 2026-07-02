#include "Character/Monsters/AI/PTMonsterAIController.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APTMonsterAIController::APTMonsterAIController()
{

}

void APTMonsterAIController::InitializeBlackboard(APawn* InPawn)
{
    if (APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(InPawn))
    {
        UpdateMonsterBlackboard(Monster);
    }
}

void APTMonsterAIController::PostPossessSetup(APawn* InPawn)
{
    APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(InPawn);
    if (!IsValid(Monster))
    {
        return;
    }

    if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
    {
        MoveComp->bUseRVOAvoidance = true;
        MoveComp->AvoidanceWeight = 0.5f;
        MoveComp->AvoidanceConsiderationRadius = 500.f;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 300.f, 0.f);
    }
}

void APTMonsterAIController::UpdateMonsterBlackboard(APTMonsterCharacter* Monster)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!IsValid(Monster) || !IsValid(BB))
    {
        return;
    }

    BB->SetValueAsVector(PTMonsterBlackboardKeys::SpawnLocation, Monster->GetSpawnLocation());
    BB->SetValueAsBool(PTMonsterBlackboardKeys::CanAttack, true);
    BB->SetValueAsBool(PTMonsterBlackboardKeys::IsRanged, Monster->IsRangedMonster());
    BB->SetValueAsFloat(PTMonsterBlackboardKeys::OptimalRange, Monster->GetOptimalRange());
}
