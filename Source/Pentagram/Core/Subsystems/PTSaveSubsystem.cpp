// Fill out your copyright notice in the Description page of Project Settings.


#include "PTSaveSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "PTPlayerLevelSubsystem.h"
#include "PTQuestSubsystem.h"
#include "PTSaveGame.h"

namespace
{
constexpr int32 PTSaveUserIndex = 0;
}

void UPTSaveSubsystem::SetPlayerSteamID(const FString& PlayerSteamID)
{
    if (PlayerSteamID.IsEmpty())
    {
        return;
    }

    SaveSlotName = FString::Printf(TEXT("PTPlayerSave_%s"), *PlayerSteamID);
    SaveData = FPTPlayerSaveData();
    bHasSaveData = false;
}

void UPTSaveSubsystem::SaveGame(const APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    PlayerStateSaveData(PlayerState);
    QuestSaveData();
    bHasSaveData = SaveSlotData();
}

void UPTSaveSubsystem::LoadGame(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    if (!LoadSlotData())
    {
        return;
    }

    PlayerStateLoadData(PlayerState);
    QuestLoadData();
}

bool UPTSaveSubsystem::HasSaveData() const
{
    return bHasSaveData || UGameplayStatics::DoesSaveGameExist(SaveSlotName, PTSaveUserIndex);
}

void UPTSaveSubsystem::DeleteSaveData()
{
    SaveData = FPTPlayerSaveData();
    bHasSaveData = false;
    UGameplayStatics::DeleteGameInSlot(SaveSlotName, PTSaveUserIndex);
}

void UPTSaveSubsystem::PlayerStateSaveData(const APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    SaveData.Gold = PlayerState->CurrentGold;
    SaveData.Level = PlayerState->PlayerLevel;
    SaveData.Exp = PlayerState->CurrentExp;
}

void UPTSaveSubsystem::QuestSaveData()
{
    UGameInstance* GameInstance = GetGameInstance();
    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem != nullptr)
    {
        SaveData.AcceptedQuests = QuestSubsystem->GetAcceptedQuestProgresses();
    }
}

void UPTSaveSubsystem::PlayerStateLoadData(APTBasePlayerState* PlayerState) const
{
    if (PlayerState == nullptr)
    {
        return;
    }

    PlayerState->CurrentGold = FMath::Max(SaveData.Gold, 0);

    UPTPlayerLevelSubsystem* PlayerLevelSubsystem =
        GetGameInstance()->GetSubsystem<UPTPlayerLevelSubsystem>();
    if (PlayerLevelSubsystem != nullptr)
    {
        PlayerLevelSubsystem->SetProgress(PlayerState, SaveData.Level, SaveData.Exp);
        return;
    }

    PlayerState->PlayerLevel = FMath::Max(SaveData.Level, 1);
    PlayerState->CurrentExp = FMath::Max(SaveData.Exp, 0);
    PlayerState->RequiredExp = FMath::Max(PlayerState->PlayerLevel, 1) * 100;
}

void UPTSaveSubsystem::QuestLoadData() const
{
    UGameInstance* GameInstance = GetGameInstance();
    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->SetAcceptedQuestProgresses(SaveData.AcceptedQuests);
    }
}

bool UPTSaveSubsystem::SaveSlotData()
{
    UPTSaveGame* SaveGameObject = Cast<UPTSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UPTSaveGame::StaticClass()));
    if (SaveGameObject == nullptr)
    {
        return false;
    }

    SaveGameObject->SaveData = SaveData;
    return UGameplayStatics::SaveGameToSlot(SaveGameObject, SaveSlotName, PTSaveUserIndex);
}

bool UPTSaveSubsystem::LoadSlotData()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, PTSaveUserIndex))
    {
        return false;
    }

    UPTSaveGame* LoadedSaveGame = Cast<UPTSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, PTSaveUserIndex));
    if (LoadedSaveGame == nullptr)
    {
        return false;
    }

    SaveData = LoadedSaveGame->SaveData;
    bHasSaveData = true;
    return true;
}
