#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PTBTService_LaserOnApproach.generated.h"

UCLASS()
class PENTAGRAM_API UPTBTService_LaserOnApproach : public UBTService
{
	GENERATED_BODY()

public:
    UPTBTService_LaserOnApproach();

    virtual uint16 GetInstanceMemorySize() const override;

protected:
    virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, Category = "PT|Boss")
    float LaserFireMaxDistance = 1500.f;
};
