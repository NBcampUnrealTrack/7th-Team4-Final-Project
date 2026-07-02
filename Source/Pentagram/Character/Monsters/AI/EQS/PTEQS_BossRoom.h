#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "PTEQS_BossRoom.generated.h"

UCLASS()
class PENTAGRAM_API UPTEQS_BossRoom : public UEnvQueryContext
{
	GENERATED_BODY()

public:
    virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
