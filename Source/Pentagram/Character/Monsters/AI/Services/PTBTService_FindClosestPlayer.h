#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PTBTService_FindClosestPlayer.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTService_FindClosestPlayer : public UBTService
{
    GENERATED_BODY()

public:
    UPTBTService_FindClosestPlayer();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "PT|AI")
    float SearchRadius = 2000.f;
};
