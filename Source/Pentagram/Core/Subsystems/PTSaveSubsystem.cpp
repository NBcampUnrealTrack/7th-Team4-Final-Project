// Fill out your copyright notice in the Description page of Project Settings.


#include "PTSaveSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Core/PTGameState.h"
#include "PTPlayerLevelSubsystem.h"
#include "PTQuestSubsystem.h"
#include "PTSaveGame.h"
#include "Misc/Paths.h"
#include "GameFramework/OnlineReplStructs.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
constexpr int32 PTSaveUserIndex = 0;
constexpr float PTAutoSaveIntervalSeconds = 60.f;
}

void UPTSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(
        this,
        &UPTSaveSubsystem::OnPreLoadMap);
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
        this,
        &UPTSaveSubsystem::OnPostLoadMapWithWorld);
}

void UPTSaveSubsystem::Deinitialize()
{
    SaveAllAuthorityPlayers(false);

    StopAutoSave();

    if (PreLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
        PreLoadMapHandle.Reset();
    }

    if (PostLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
        PostLoadMapHandle.Reset();
    }

    Super::Deinitialize();
}

bool UPTSaveSubsystem::SavePlayer(const APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return false;
    }

    const FString PlayerSaveID = GetPlayerSaveID(PlayerState);
    if (PlayerSaveID.IsEmpty())
    {
        return false;
    }

    return WriteSlotDataToSlot(MakeSaveSlotName(PlayerSaveID), CaptureFromPlayerState(PlayerState));
}

bool UPTSaveSubsystem::LoadPlayer(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return false;
    }

    const FString PlayerSaveID = GetPlayerSaveID(PlayerState);
    if (PlayerSaveID.IsEmpty())
    {
        return false;
    }

    FPTPlayerSaveData PlayerSaveData;
    if (!ReadSlotDataFromSlot(MakeSaveSlotName(PlayerSaveID), PlayerSaveData))
    {
        return false;
    }

    ApplyToPlayerState(PlayerState, PlayerSaveData);
    return true;
}

bool UPTSaveSubsystem::SaveAllAuthorityPlayers(bool bSkipBossFight)
{
    if (bSkipBossFight && ShouldSkipAutoSave())
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (World == nullptr || World->GetNetMode() == NM_Client)
    {
        return false;
    }

    AGameStateBase* GameState = World->GetGameState();
    if (GameState == nullptr)
    {
        return false;
    }

    bool bSavedAnyPlayer = false;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APTBasePlayerState* PTPlayerState = Cast<APTBasePlayerState>(PlayerState);
        if (PTPlayerState != nullptr)
        {
            bSavedAnyPlayer |= SavePlayer(PTPlayerState);
        }
    }

    return bSavedAnyPlayer;
}

FPTPlayerSaveData UPTSaveSubsystem::CaptureFromPlayerState(const APTBasePlayerState* PlayerState) const
{
    FPTPlayerSaveData PlayerSaveData;

    if (PlayerState == nullptr)
    {
        return PlayerSaveData;
    }

    PlayerSaveData.Gold = PlayerState->CurrentGold;
    PlayerSaveData.Level = PlayerState->PlayerLevel;
    PlayerSaveData.Exp = PlayerState->CurrentExp;

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance != nullptr)
    {
        UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
        if (QuestSubsystem != nullptr)
        {
            PlayerSaveData.AcceptedQuests = QuestSubsystem->GetAcceptedQuestProgresses(PlayerState);
        }
    }

    return PlayerSaveData;
}

void UPTSaveSubsystem::ApplyToPlayerState(APTBasePlayerState* PlayerState, const FPTPlayerSaveData& PlayerSaveData) const
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return;
    }

    PlayerState->CurrentGold = FMath::Max(PlayerSaveData.Gold, 0);

    UGameInstance* GameInstance = GetGameInstance();
    UPTPlayerLevelSubsystem* PlayerLevelSubsystem = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UPTPlayerLevelSubsystem>()
        : nullptr;
    if (PlayerLevelSubsystem != nullptr)
    {
        PlayerLevelSubsystem->SetProgress(PlayerState, PlayerSaveData.Level, PlayerSaveData.Exp);
    }
    else
    {
        PlayerState->PlayerLevel = FMath::Max(PlayerSaveData.Level, 1);
        PlayerState->CurrentExp = FMath::Max(PlayerSaveData.Exp, 0);
        PlayerState->RequiredExp = FMath::Max(PlayerState->PlayerLevel, 1) * 100;
        PlayerState->OnLevelChanged.Broadcast(PlayerState->PlayerLevel);
        PlayerState->OnExpChanged.Broadcast(PlayerState->CurrentExp, PlayerState->RequiredExp);
    }

    PlayerState->OnGoldChanged.Broadcast(PlayerState->CurrentGold);

    if (GameInstance != nullptr)
    {
        UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
        if (QuestSubsystem != nullptr)
        {
            QuestSubsystem->SetAcceptedQuestProgresses(PlayerState, PlayerSaveData.AcceptedQuests);
        }
    }
}

bool UPTSaveSubsystem::HasPlayerSaveData(const APTBasePlayerState* PlayerState) const
{
    const FString PlayerSaveID = GetPlayerSaveID(PlayerState);
    if (PlayerSaveID.IsEmpty())
    {
        return false;
    }

    return UGameplayStatics::DoesSaveGameExist(MakeSaveSlotName(PlayerSaveID), PTSaveUserIndex);
}

bool UPTSaveSubsystem::WriteSlotDataToSlot(const FString& SlotName, const FPTPlayerSaveData& PlayerSaveData)
{
    if (SlotName.IsEmpty())
    {
        return false;
    }

    UPTSaveGame* SaveGameObject = Cast<UPTSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UPTSaveGame::StaticClass()));
    if (SaveGameObject == nullptr)
    {
        return false;
    }

    SaveGameObject->SaveData = PlayerSaveData;
    return UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, PTSaveUserIndex);
}

bool UPTSaveSubsystem::ReadSlotDataFromSlot(const FString& SlotName, FPTPlayerSaveData& OutPlayerSaveData)
{
    if (SlotName.IsEmpty())
    {
        return false;
    }

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, PTSaveUserIndex))
    {
        return false;
    }

    UPTSaveGame* LoadedSaveGame = Cast<UPTSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, PTSaveUserIndex));
    if (LoadedSaveGame == nullptr)
    {
        return false;
    }

    OutPlayerSaveData = LoadedSaveGame->SaveData;
    return true;
}

void UPTSaveSubsystem::StartAutoSave()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    if (!World->GetTimerManager().IsTimerActive(AutoSaveTimerHandle))
    {
        World->GetTimerManager().SetTimer(
            AutoSaveTimerHandle,
            this,
            &UPTSaveSubsystem::OnAutoSaveTimer,
            PTAutoSaveIntervalSeconds,
            true,
            PTAutoSaveIntervalSeconds);
    }
}

void UPTSaveSubsystem::StopAutoSave()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
}

void UPTSaveSubsystem::OnAutoSaveTimer()
{
    SaveAllAuthorityPlayers(true);
}

void UPTSaveSubsystem::OnPreLoadMap(const FString& MapName)
{
    SaveAllAuthorityPlayers(false);

    StopAutoSave();
}

void UPTSaveSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
    StartAutoSave();
}

FString UPTSaveSubsystem::MakeSaveSlotName(const FString& PlayerSaveID) const
{
    if (PlayerSaveID.IsEmpty())
    {
        return FString();
    }

    return FString::Printf(TEXT("PTPlayerSave_%s"), *FPaths::MakeValidFileName(PlayerSaveID));
}

FString UPTSaveSubsystem::GetPlayerSaveID(const APTBasePlayerState* PlayerState) const
{
    if (PlayerState == nullptr)
    {
        return FString();
    }

    const FUniqueNetIdRepl& UniqueID = PlayerState->GetUniqueId();
    if (UniqueID.IsValid())
    {
        return UniqueID.ToString();
    }

    const int32 PlayerID = PlayerState->GetPlayerId();
    if (PlayerID >= 0)
    {
        return FString::Printf(TEXT("PlayerId_%d"), PlayerID);
    }

    if (!PlayerState->GetPlayerName().IsEmpty())
    {
        return PlayerState->GetPlayerName();
    }

    return FString();
}

bool UPTSaveSubsystem::ShouldSkipAutoSave() const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return false;
    }

    const APTGameState* GameState = World->GetGameState<APTGameState>();
    return GameState != nullptr && GameState->GetCurrentPhase() == EGamePhase::BossFight;
}
