#include "Character/Monsters/AI/EQS/PTEQS_Target.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UPTEQS_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
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
        UE_LOG(LogTemp, Warning, TEXT("[PTEQS_Target] AIC null. Owner=%s"), *GetNameSafe(QueryInstance.Owner.Get()));
        return;
    }

    UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (!IsValid(BB))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTEQS_Target] BB null"));
        return;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
    if (!IsValid(Target))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTEQS_Target] TargetActor null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[PTEQS_Target] OK Target=%s"), *GetNameSafe(Target));
    UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
}
