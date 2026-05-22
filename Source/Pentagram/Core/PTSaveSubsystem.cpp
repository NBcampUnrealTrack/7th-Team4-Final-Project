// Fill out your copyright notice in the Description page of Project Settings.


#include "PTSaveSubsystem.h"

void UPTSaveSubsystem::SaveGame()
{
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }
}

void UPTSaveSubsystem::LoadGame()
{
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }


}
