// Fill out your copyright notice in the Description page of Project Settings.


#include "PTEconomySubsystem.h"

void UPTEconomySubsystem::AddGold(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }

    CurrentGold += Amount;
}

bool UPTEconomySubsystem::SpendGold(int32 Amount)
{
    if (Amount <= 0)
    {
        return false;
    }
    if (CurrentGold < Amount)
    {
        return false;
    }

    CurrentGold -= Amount;
    return true;
}

int32 UPTEconomySubsystem::GetGold() const
{
    return CurrentGold;
}
