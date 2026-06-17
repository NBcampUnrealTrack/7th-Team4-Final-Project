// Fill out your copyright notice in the Description page of Project Settings.


#include "PTSaveSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
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
constexpr float PTLoadRetryIntervalSeconds = 1.f;
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
    if (!SaveAllAuthorityPlayers(false))
    {
        SaveLocalPlayer(false);
    }

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

void UPTSaveSubsystem::SetPlayerSteamID(const FString& PlayerSteamID)
{
    if (PlayerSteamID.IsEmpty())
    {
        return;
    }

    const FString NewSaveSlotName = MakeSaveSlotName(PlayerSteamID);
    if (!SaveSlotName.IsEmpty() && SaveSlotName != NewSaveSlotName)
    {
        SaveLocalPlayer(false);
    }

    StopAutoSave();

    SaveSlotName = NewSaveSlotName;
    SaveData = FPTPlayerSaveData();
    bHasSaveData = false;
    bHasLoadedLocalPlayer = false;

    StartAutoSave();
    TryLoadLocalPlayer();
}

void UPTSaveSubsystem::SaveGame(const APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    if (PlayerState->HasAuthority())
    {
        WriteSlotData(CaptureFromPlayerState(PlayerState));
    }
}

void UPTSaveSubsystem::LoadGame(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr)
    {
        return;
    }

    if (!PlayerState->HasAuthority())
    {
        return;
    }

    FPTPlayerSaveData PlayerSaveData;
    if (!ReadSlotData(PlayerSaveData))
    {
        return;
    }

    ApplyToPlayerState(PlayerState, PlayerSaveData);
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

bool UPTSaveSubsystem::SaveLocalPlayer(bool bSkipBossFight)
{
    if (SaveSlotName.IsEmpty())
    {
        return false;
    }

    if (bSkipBossFight && ShouldSkipAutoSave())
    {
        return false;
    }

    const APTBasePlayerState* PlayerState = GetLocalPlayerState();
    if (PlayerState == nullptr)
    {
        return false;
    }

    if (!PlayerState->HasAuthority())
    {
        return false;
    }

    return WriteSlotData(CaptureFromPlayerState(PlayerState));
}

bool UPTSaveSubsystem::LoadLocalPlayer()
{
    if (SaveSlotName.IsEmpty())
    {
        return false;
    }

    APTBasePlayerState* PlayerState = GetLocalPlayerState();
    if (PlayerState == nullptr)
    {
        return false;
    }

    if (!PlayerState->HasAuthority())
    {
        bHasLoadedLocalPlayer = true;
        return true;
    }

    if (!HasSaveData())
    {
        bHasLoadedLocalPlayer = true;
        return true;
    }

    FPTPlayerSaveData PlayerSaveData;
    if (!ReadSlotData(PlayerSaveData))
    {
        return false;
    }

    ApplyToPlayerState(PlayerState, PlayerSaveData);
    bHasLoadedLocalPlayer = true;
    return true;
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

bool UPTSaveSubsystem::WriteSlotData(const FPTPlayerSaveData& PlayerSaveData)
{
    if (SaveSlotName.IsEmpty())
    {
        return false;
    }

    return WriteSlotDataToSlot(SaveSlotName, PlayerSaveData);
}

bool UPTSaveSubsystem::ReadSlotData(FPTPlayerSaveData& OutPlayerSaveData)
{
    if (SaveSlotName.IsEmpty())
    {
        return false;
    }

    return ReadSlotDataFromSlot(SaveSlotName, OutPlayerSaveData);
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

bool UPTSaveSubsystem::HasSaveData() const
{
    if (SaveSlotName.IsEmpty())
    {
        return false;
    }

    return bHasSaveData || UGameplayStatics::DoesSaveGameExist(SaveSlotName, PTSaveUserIndex);
}

void UPTSaveSubsystem::DeleteSaveData()
{
    if (SaveSlotName.IsEmpty())
    {
        return;
    }

    SaveData = FPTPlayerSaveData();
    bHasSaveData = false;
    UGameplayStatics::DeleteGameInSlot(SaveSlotName, PTSaveUserIndex);
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
    const bool bWroteSaveData = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, PTSaveUserIndex);
    if (bWroteSaveData && SlotName == SaveSlotName)
    {
        SaveData = PlayerSaveData;
        bHasSaveData = true;
    }

    return bWroteSaveData;
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
    if (SlotName == SaveSlotName)
    {
        SaveData = OutPlayerSaveData;
        bHasSaveData = true;
    }

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
    World->GetTimerManager().ClearTimer(LoadRetryTimerHandle);
}

void UPTSaveSubsystem::TryLoadLocalPlayer()
{
    if (bHasLoadedLocalPlayer)
    {
        return;
    }

    if (LoadLocalPlayer())
    {
        UWorld* World = GetWorld();
        if (World == nullptr)
        {
            return;
        }

        World->GetTimerManager().ClearTimer(LoadRetryTimerHandle);
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr || SaveSlotName.IsEmpty())
    {
        return;
    }

    if (!World->GetTimerManager().IsTimerActive(LoadRetryTimerHandle))
    {
        World->GetTimerManager().SetTimer(
            LoadRetryTimerHandle,
            this,
            &UPTSaveSubsystem::TryLoadLocalPlayer,
            PTLoadRetryIntervalSeconds,
            true);
    }
}

void UPTSaveSubsystem::OnAutoSaveTimer()
{
    if (!SaveAllAuthorityPlayers(true))
    {
        SaveLocalPlayer(true);
    }
}

void UPTSaveSubsystem::OnPreLoadMap(const FString& MapName)
{
    if (!SaveAllAuthorityPlayers(false))
    {
        SaveLocalPlayer(false);
    }

    bHasLoadedLocalPlayer = false;
    StopAutoSave();
}

void UPTSaveSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
    bHasLoadedLocalPlayer = false;
    StartAutoSave();
    TryLoadLocalPlayer();
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

APlayerController* UPTSaveSubsystem::GetLocalPlayerController() const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return nullptr;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController == nullptr || !PlayerController->IsLocalPlayerController())
    {
        return nullptr;
    }

    return PlayerController;
}

APTBasePlayerState* UPTSaveSubsystem::GetLocalPlayerState() const
{
    APlayerController* PlayerController = GetLocalPlayerController();
    if (PlayerController == nullptr)
    {
        return nullptr;
    }

    return PlayerController->GetPlayerState<APTBasePlayerState>();
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
