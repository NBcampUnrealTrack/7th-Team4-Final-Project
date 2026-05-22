// Fill out your copyright notice in the Description page of Project Settings.


#include "PTEconomySubsystem.h"

void UPTEconomySubsystem::AddGold(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }


}

void UPTEconomySubsystem::SpendGold(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }
    /*if (//현재 골드 < Amount)
    {
        return;
    }*/
}

int32 UPTEconomySubsystem::GetGold()
{
    return 0;  //현재 골드로 나중에 변경
}
