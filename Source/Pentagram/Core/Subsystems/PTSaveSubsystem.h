#pragma once

#include "CoreMinimal.h"
#include "Core/PTQuestDataRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTSaveSubsystem.generated.h"

class APTBasePlayerState;

USTRUCT(BlueprintType)
struct FPTPlayerSaveData
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
    UFUNCTION(BlueprintCallable, Category = "PT|Save")
    void SetPlayerSteamID(const FString& PlayerSteamID);

    void SaveGame(const APTBasePlayerState* PlayerState);
    void LoadGame(APTBasePlayerState* PlayerState);
    bool HasSaveData() const;
    void DeleteSaveData();
    const FPTPlayerSaveData& GetSaveData() const { return SaveData; }

private:
    void PlayerStateSaveData(const APTBasePlayerState* PlayerState);
    void QuestSaveData();
    void PlayerStateLoadData(APTBasePlayerState* PlayerState) const;
    void QuestLoadData() const;
    bool SaveSlotData();
    bool LoadSlotData();

    FPTPlayerSaveData SaveData;
    FString SaveSlotName = TEXT("PTPlayerSave");
    bool bHasSaveData = false;
};
