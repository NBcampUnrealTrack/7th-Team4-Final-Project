// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTEconomySubsystem.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTEconomySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    void AddGold(int32 Amount);
    void SpendGold(int32 Amount);
    int32 GetGold();
};
