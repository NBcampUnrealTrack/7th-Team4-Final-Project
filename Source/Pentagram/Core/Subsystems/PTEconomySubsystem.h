#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTEconomySubsystem.generated.h"

class APTBasePlayerState;

DECLARE_MULTICAST_DELEGATE_TwoParams(FPTNativeOnGoldChanged, APTBasePlayerState*, int32);

UCLASS()
class PENTAGRAM_API UPTEconomySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void AddGold(APTBasePlayerState* PlayerState, int32 Amount);
    bool SpendGold(APTBasePlayerState* PlayerState, int32 Amount);
    void SetGold(APTBasePlayerState* PlayerState, int32 Amount);
    int32 GetGold(const APTBasePlayerState* PlayerState) const;
    bool CanAfford(const APTBasePlayerState* PlayerState, int32 Amount) const;

    FPTNativeOnGoldChanged OnGoldChanged;
};
