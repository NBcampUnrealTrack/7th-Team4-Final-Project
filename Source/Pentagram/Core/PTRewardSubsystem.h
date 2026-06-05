#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PTRewardSubsystem.generated.h"

class APTMonsterCharacter;
class APTBasePlayerState;

UCLASS()
class PENTAGRAM_API UPTRewardSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void HandleMonsterDeathReward(APTMonsterCharacter* DeadMonster);

private:
    void GiveExpToContributors(APTMonsterCharacter* DeadMonster);

    void SpawnDeathDrops(APTMonsterCharacter* DeadMonster);

    bool HasServerAuthority() const;
};
