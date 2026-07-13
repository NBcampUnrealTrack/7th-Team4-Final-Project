#pragma once

#include "CoreMinimal.h"
#include "Character/Player/PTEquipmentComponent.h"
#include "Core/PTQuestDataRow.h"
#include "Engine/TimerHandle.h"
#include "Item/PTItemTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTSaveSubsystem.generated.h"

class APTBasePlayerState;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    int32 SaveVersion = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    bool bHasInventoryData = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    TArray<FInventorySlot> InventorySlots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    bool bHasEquipmentData = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    TArray<FEquipmentSlot> EquipmentSlots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    bool bHasSkillData = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    TArray<FName> LearnedSkills;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Save")
    TArray<FName> SkillSlots;
};


UCLASS()
class PENTAGRAM_API UPTSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    bool SavePlayer(const APTBasePlayerState* PlayerState);
    bool LoadPlayer(APTBasePlayerState* PlayerState);
    bool SaveAllAuthorityPlayers(bool bSkipBossFight);
    FPTPlayerSaveData CaptureFromPlayerState(const APTBasePlayerState* PlayerState) const;
    void ApplyToPlayerState(APTBasePlayerState* PlayerState, const FPTPlayerSaveData& PlayerSaveData) const;
    bool ApplyPendingPlayerCharacterData(APTBasePlayerState* PlayerState);
    bool HasPlayerSaveData(const APTBasePlayerState* PlayerState) const;
    void NotifyWorldReadyForAutoSave();

private:
    void StartAutoSave();
    void StopAutoSave();
    void OnAutoSaveTimer();
    void OnPreLoadMap(const FString& MapName);
    void OnPostLoadMapWithWorld(UWorld* LoadedWorld);
    FString MakeSaveSlotName(const FString& PlayerSaveID) const;
    FString GetPlayerSaveID(const APTBasePlayerState* PlayerState) const;
    bool WriteSlotDataToSlot(const FString& SlotName, const FPTPlayerSaveData& PlayerSaveData);
    bool ReadSlotDataFromSlot(const FString& SlotName, FPTPlayerSaveData& OutPlayerSaveData);
    bool ApplyToPlayerCharacter(APTBasePlayerState* PlayerState, const FPTPlayerSaveData& PlayerSaveData) const;
    void MergeCharacterDataFromExistingSave(FPTPlayerSaveData& PlayerSaveData, const FPTPlayerSaveData& ExistingSaveData) const;
    bool ShouldSkipAutoSave() const;

    FTimerHandle AutoSaveTimerHandle;
    FDelegateHandle PreLoadMapHandle;
    FDelegateHandle PostLoadMapHandle;
    TMap<FString, FPTPlayerSaveData> PendingPlayerCharacterData;
};
