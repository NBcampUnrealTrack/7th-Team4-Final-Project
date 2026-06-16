#pragma once

#include "CoreMinimal.h"
#include "Core/PTQuestDataRow.h"
#include "Engine/TimerHandle.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTSaveSubsystem.generated.h"

class APTBasePlayerState;
class APlayerController;
class UWorld;

USTRUCT(BlueprintType)
struct PENTAGRAM_API FPTPlayerSaveData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    int32 Gold = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    int32 Level = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    int32 Exp = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
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
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "PT|Save")
    void SetPlayerSteamID(const FString& PlayerSteamID);

    void SaveGame(const APTBasePlayerState* PlayerState);
    void LoadGame(APTBasePlayerState* PlayerState);
    bool SavePlayer(const APTBasePlayerState* PlayerState);
    bool LoadPlayer(APTBasePlayerState* PlayerState);
    bool SaveAllAuthorityPlayers(bool bSkipBossFight);
    bool SaveLocalPlayer(bool bSkipBossFight);
    bool LoadLocalPlayer();
    FPTPlayerSaveData CaptureFromPlayerState(const APTBasePlayerState* PlayerState) const;
    void ApplyToPlayerState(APTBasePlayerState* PlayerState, const FPTPlayerSaveData& PlayerSaveData) const;
    bool WriteSlotData(const FPTPlayerSaveData& PlayerSaveData);
    bool ReadSlotData(FPTPlayerSaveData& OutPlayerSaveData);
    bool HasPlayerSaveData(const APTBasePlayerState* PlayerState) const;
    bool HasSaveData() const;
    void DeleteSaveData();
    const FPTPlayerSaveData& GetSaveData() const { return SaveData; }

private:
    void StartAutoSave();
    void StopAutoSave();
    void TryLoadLocalPlayer();
    void OnAutoSaveTimer();
    void OnPreLoadMap(const FString& MapName);
    void OnPostLoadMapWithWorld(UWorld* LoadedWorld);
    FString MakeSaveSlotName(const FString& PlayerSaveID) const;
    FString GetPlayerSaveID(const APTBasePlayerState* PlayerState) const;
    bool WriteSlotDataToSlot(const FString& SlotName, const FPTPlayerSaveData& PlayerSaveData);
    bool ReadSlotDataFromSlot(const FString& SlotName, FPTPlayerSaveData& OutPlayerSaveData);
    APlayerController* GetLocalPlayerController() const;
    APTBasePlayerState* GetLocalPlayerState() const;
    bool ShouldSkipAutoSave() const;

    FPTPlayerSaveData SaveData;
    FString SaveSlotName;
    FTimerHandle AutoSaveTimerHandle;
    FTimerHandle LoadRetryTimerHandle;
    FDelegateHandle PreLoadMapHandle;
    FDelegateHandle PostLoadMapHandle;
    bool bHasLoadedLocalPlayer = false;
    bool bHasSaveData = false;
};
