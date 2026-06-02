// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTEconomySubsystem.generated.h"

class APTBasePlayerState;

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTEconomySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void AddGold(APTBasePlayerState* PlayerState, int32 Amount);
    bool SpendGold(APTBasePlayerState* PlayerState, int32 Amount);
    int32 GetGold(const APTBasePlayerState* PlayerState) const;
    bool CanAfford(const APTBasePlayerState* PlayerState, int32 Amount) const;
};
