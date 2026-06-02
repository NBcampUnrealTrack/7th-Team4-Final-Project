// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTQuestDataRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTSaveSubsystem.generated.h"

class APTBasePlayerState;

USTRUCT(BlueprintType)
struct FPTSaveData
{
    GENERATED_BODY()

    UPROPERTY()
    int32 Gold = 0;

    UPROPERTY()
    int32 Level = 1;

    UPROPERTY()
    int32 Exp = 0;

    UPROPERTY()
    TArray<FPTQuestProgress> AcceptedQuests;

    //TArray<FInventoryItem> Inventory;
    //TArray<FEquipSlot> Equipment;
    //TArray<FSkillSlot> SkillSlots;
};


UCLASS()
class PENTAGRAM_API UPTSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void SaveGame();
    void SaveGame(const APTBasePlayerState* PlayerState);
    void LoadGame();
    void LoadGame(APTBasePlayerState* PlayerState);
    bool HasSaveData() const;
    void DeleteSaveData();
    const FPTSaveData& GetSaveData() const { return SaveData; }

private:
    void PlayerStateSaveData(const APTBasePlayerState* PlayerState);
    void QuestSaveData();
    void PlayerStateLoadData(APTBasePlayerState* PlayerState) const;
    void QuestLoadData() const;

    FPTSaveData SaveData; // save 데이터
    bool bHasSaveData = false;
};
