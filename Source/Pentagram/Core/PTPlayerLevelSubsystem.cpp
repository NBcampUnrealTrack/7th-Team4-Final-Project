// Fill out your copyright notice in the Description page of Project Settings.


#include "PTPlayerLevelSubsystem.h"

void UPTPlayerLevelSubsystem::AddExp(int32 ExpAmount)
{
    if (ExpAmount <= 0)
    {
        return;
    }

}

void UPTPlayerLevelSubsystem::LevelUp()
{

}

int32 UPTPlayerLevelSubsystem::GetLevel()
{
    return 0;  // 현재 레벨하면 됨.
}

int32 UPTPlayerLevelSubsystem::GetExp()
{
    return 0;   //현재 exp하면 됨.
}

void UPTPlayerLevelSubsystem::ApplyDeathPenalty()
{
        //데스페널티 만들면 됨.
}
