#include "Character/Monsters/AI/EQS/PTEQS_Home.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

void UPTEQS_Home::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
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

    const FVector Home = BB->GetValueAsVector(PTMonsterBlackboardKeys::SpawnLocation);
    UEnvQueryItemType_Point::SetContextHelper(ContextData, Home);
}
