#pragma once

#include "CoreMinimal.h"
#include "Character/Monsters/AI/Controllers/PTBaseAIController.h"
#include "PTBossAIController.generated.h"

class APTBossMonsterCharacter;

UCLASS()
class PENTAGRAM_API APTBossAIController : public APTBaseAIController
{
	GENERATED_BODY()

public:
    APTBossAIController();

protected:
    virtual void InitializeBlackboard(APawn* InPawn) override;
    virtual void PostPossessSetup(APawn* InPawn) override;

private:
    UFUNCTION()
    void OnBossHPChanged(float CurrentHP, float MaxHP);

    int32 CachedPhase = 0;
};
