// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTSaveSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FPTSaveData
{
    GENERATED_BODY()

    int32 Gold;
    int32 Level;
    int32 Exp;
    //TArray<FInventoryItem> Inventory;
    //TArray<FEquipSlot> Equipment;
    //TArray<FSkillSlot> SkillSlots;
};


UCLASS()
class PENTAGRAM_API UPTSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    FPTSaveData SaveData; // save 데이터

    void SaveGame();
    void LoadGame();
};
