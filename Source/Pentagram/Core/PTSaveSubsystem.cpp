// Fill out your copyright notice in the Description page of Project Settings.


#include "PTSaveSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "PTPlayerLevelSubsystem.h"
#include "PTQuestSubsystem.h"
#include "PTSaveGame.h"

namespace
{
const FString PTSaveSlotName = TEXT("PTPlayerSave");
constexpr int32 PTSaveUserIndex = 0;
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
    return bHasSaveData || UGameplayStatics::DoesSaveGameExist(PTSaveSlotName, PTSaveUserIndex);
}

void UPTSaveSubsystem::DeleteSaveData()
{
    SaveData = FPTPlayerSaveData();
    bHasSaveData = false;
    UGameplayStatics::DeleteGameInSlot(PTSaveSlotName, PTSaveUserIndex);
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
    return UGameplayStatics::SaveGameToSlot(SaveGameObject, PTSaveSlotName, PTSaveUserIndex);
}

bool UPTSaveSubsystem::LoadSlotData()
{
    if (!UGameplayStatics::DoesSaveGameExist(PTSaveSlotName, PTSaveUserIndex))
    {
        return false;
    }

    UPTSaveGame* LoadedSaveGame = Cast<UPTSaveGame>(
        UGameplayStatics::LoadGameFromSlot(PTSaveSlotName, PTSaveUserIndex));
    if (LoadedSaveGame == nullptr)
    {
        return false;
    }

    SaveData = LoadedSaveGame->SaveData;
    bHasSaveData = true;
    return true;
}
