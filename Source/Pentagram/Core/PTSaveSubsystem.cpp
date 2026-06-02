// Fill out your copyright notice in the Description page of Project Settings.


#include "PTSaveSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "PTQuestSubsystem.h"

void UPTSaveSubsystem::SaveGame()
{
    QuestSaveData();
    bHasSaveData = true;
}

void UPTSaveSubsystem::SaveGame(const APTBasePlayerState* PlayerState)
{
    PlayerStateSaveData(PlayerState);
    QuestSaveData();
    bHasSaveData = true;
}

void UPTSaveSubsystem::LoadGame()
{
    if (!bHasSaveData)
    {
        return;
    }

    QuestLoadData();
}

void UPTSaveSubsystem::LoadGame(APTBasePlayerState* PlayerState)
{
    if (!bHasSaveData)
    {
        return;
    }

    PlayerStateLoadData(PlayerState);
    QuestLoadData();
}

bool UPTSaveSubsystem::HasSaveData() const
{
    return bHasSaveData;
}

void UPTSaveSubsystem::DeleteSaveData()
{
    SaveData = FPTSaveData();
    bHasSaveData = false;
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
    if (GameInstance == nullptr)
    {
        return;
    }

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
    PlayerState->PlayerLevel = FMath::Max(SaveData.Level, 1);
    PlayerState->CurrentExp = FMath::Max(SaveData.Exp, 0);
    PlayerState->RequiredExp = FMath::Max(PlayerState->PlayerLevel, 1) * 100;
}

void UPTSaveSubsystem::QuestLoadData() const
{
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->SetAcceptedQuestProgresses(SaveData.AcceptedQuests);
    }
}
