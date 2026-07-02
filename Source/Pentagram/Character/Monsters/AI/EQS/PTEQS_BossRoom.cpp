#include "Character/Monsters/AI/EQS/PTEQS_BossRoom.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

void UPTEQS_BossRoom::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
    AAIController* AIC = Cast<AAIController>(QueryInstance.Owner.Get());
    if (!IsValid(AIC))
    {
        APawn* Pawn = Cast<APawn>(QueryInstance.Owner.Get());
        if (IsValid(Pawn))
        {
            AIC = Cast<AAIController>(Pawn->GetController());
        }
    }

    if (!IsValid(AIC))
    {
        return;
    }

    UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return;
    }

    const FVector RoomCenter = BB->GetValueAsVector(PTMonsterBlackboardKeys::SpawnLocation);
    UEnvQueryItemType_Point::SetContextHelper(ContextData, RoomCenter);
}
