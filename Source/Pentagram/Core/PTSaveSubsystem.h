// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTSaveSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FPTSaveData
{
    GENERATED_BODY()

    int32 Gold =0;
    int32 Level = 1;
    int32 Exp = 0;;
    //TArray<FInventoryItem> Inventory;
    //TArray<FEquipSlot> Equipment;
    //TArray<FSkillSlot> SkillSlots;
};


UCLASS()
class PENTAGRAM_API UPTSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    FPTSaveData SaveData; // save 데이터

    void SaveGame();    //세이브
    void LoadGame();    //로드
};
