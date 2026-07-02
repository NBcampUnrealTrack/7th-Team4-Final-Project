#pragma once

#include "CoreMinimal.h"
#include "Character/Monsters/AI/Controllers/PTBaseAIController.h"
#include "PTMonsterAIController.generated.h"

class APTMonsterCharacter;

UCLASS()
class PENTAGRAM_API APTMonsterAIController : public APTBaseAIController
{
	GENERATED_BODY()

public:
    APTMonsterAIController();

    void UpdateMonsterBlackboard(APTMonsterCharacter* Monster);

protected:
    virtual void InitializeBlackboard(APawn* InPawn) override;
    virtual void PostPossessSetup(APawn* InPawn) override;
};
