// Fill out your copyright notice in the Description page of Project Settings.


#include "PTSaveSubsystem.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTInventoryComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
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
constexpr int32 PTCurrentSaveVersion = 2;
constexpr float PTAutoSaveIntervalSeconds = 60.f;

int32 CountOccupiedInventorySlots(const FPTPlayerSaveData& PlayerSaveData)
{
    int32 OccupiedSlotCount = 0;
    for (const FInventorySlot& InventorySlot : PlayerSaveData.InventorySlots)
    {
        if (!InventorySlot.IsEmpty())
        {
            ++OccupiedSlotCount;
        }
    }

    return OccupiedSlotCount;
}

void CountQuestStates(
    const FPTPlayerSaveData& PlayerSaveData,
    int32& OutInProgressCount,
    int32& OutCompletedCount,
    int32& OutRewardedCount)
{
    OutInProgressCount = 0;
    OutCompletedCount = 0;
    OutRewardedCount = 0;

    for (const FPTQuestProgress& QuestProgress : PlayerSaveData.AcceptedQuests)
    {
        switch (QuestProgress.State)
        {
        case EPTQuestProgressState::InProgress:
            ++OutInProgressCount;
            break;
        case EPTQuestProgressState::Completed:
            ++OutCompletedCount;
            break;
        case EPTQuestProgressState::Rewarded:
            ++OutRewardedCount;
            break;
        }
    }
}
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

    const FString SlotName = MakeSaveSlotName(PlayerSaveID);
    FPTPlayerSaveData PlayerSaveData = CaptureFromPlayerState(PlayerState);

    if (!PlayerSaveData.bHasInventoryData ||
        !PlayerSaveData.bHasEquipmentData ||
        !PlayerSaveData.bHasSkillData)
    {
        FPTPlayerSaveData ExistingSaveData;
        if (ReadSlotDataFromSlot(SlotName, ExistingSaveData))
        {
            MergeCharacterDataFromExistingSave(PlayerSaveData, ExistingSaveData);
        }
    }

    const bool bHasCharacterData = PlayerSaveData.bHasInventoryData ||
        PlayerSaveData.bHasEquipmentData ||
        PlayerSaveData.bHasSkillData;
    if (bHasCharacterData)
    {
        PendingPlayerCharacterData.Add(PlayerSaveID, PlayerSaveData);
    }

    const bool bSaved = WriteSlotDataToSlot(SlotName, PlayerSaveData);
    int32 InProgressQuestCount = 0;
    int32 CompletedQuestCount = 0;
    int32 RewardedQuestCount = 0;
    CountQuestStates(
        PlayerSaveData,
        InProgressQuestCount,
        CompletedQuestCount,
        RewardedQuestCount);
    if (bSaved)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("[Save] Player save completed. PlayerID=%s OccupiedInventorySlots=%d TotalInventorySlots=%d Quests=%d/%d/%d"),
            *PlayerSaveID,
            CountOccupiedInventorySlots(PlayerSaveData),
            PlayerSaveData.InventorySlots.Num(),
            InProgressQuestCount,
            CompletedQuestCount,
            RewardedQuestCount);
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Save] Player save failed. PlayerID=%s OccupiedInventorySlots=%d TotalInventorySlots=%d Quests=%d/%d/%d"),
            *PlayerSaveID,
            CountOccupiedInventorySlots(PlayerSaveData),
            PlayerSaveData.InventorySlots.Num(),
            InProgressQuestCount,
            CompletedQuestCount,
            RewardedQuestCount);
    }
    return bSaved;
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

    PendingPlayerCharacterData.Remove(PlayerSaveID);

    FPTPlayerSaveData PlayerSaveData;
    if (!ReadSlotDataFromSlot(MakeSaveSlotName(PlayerSaveID), PlayerSaveData))
    {
        return false;
    }

    int32 InProgressQuestCount = 0;
    int32 CompletedQuestCount = 0;
    int32 RewardedQuestCount = 0;
    CountQuestStates(
        PlayerSaveData,
        InProgressQuestCount,
        CompletedQuestCount,
        RewardedQuestCount);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Save] Player save loaded. PlayerID=%s OccupiedInventorySlots=%d TotalInventorySlots=%d Quests=%d/%d/%d"),
        *PlayerSaveID,
        CountOccupiedInventorySlots(PlayerSaveData),
        PlayerSaveData.InventorySlots.Num(),
        InProgressQuestCount,
        CompletedQuestCount,
        RewardedQuestCount);

    ApplyToPlayerState(PlayerState, PlayerSaveData);

    const bool bHasCharacterData = PlayerSaveData.bHasInventoryData ||
        PlayerSaveData.bHasEquipmentData ||
        PlayerSaveData.bHasSkillData;
    if (bHasCharacterData)
    {
        PendingPlayerCharacterData.Add(PlayerSaveID, PlayerSaveData);
        ApplyPendingPlayerCharacterData(PlayerState);
    }

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
    PlayerSaveData.SaveVersion = PTCurrentSaveVersion;

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

    const AController* Controller = Cast<AController>(PlayerState->GetOwner());
    const APTPlayerCharacter* PlayerCharacter =
        Controller != nullptr ? Cast<APTPlayerCharacter>(Controller->GetPawn()) : nullptr;
    if (PlayerCharacter == nullptr)
    {
        return PlayerSaveData;
    }

    const UPTInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent();
    if (InventoryComponent != nullptr)
    {
        PlayerSaveData.bHasInventoryData = true;
        PlayerSaveData.InventorySlots = InventoryComponent->GetInventorySlots();
    }

    const UPTEquipmentComponent* EquipmentComponent = PlayerCharacter->GetEquipmentComponent();
    if (EquipmentComponent != nullptr)
    {
        PlayerSaveData.bHasEquipmentData = true;
        PlayerSaveData.EquipmentSlots = EquipmentComponent->GetEquipmentSlots();
    }

    const UPTPlayerSkillComponent* SkillComponent = PlayerCharacter->SkillComp;
    if (SkillComponent != nullptr)
    {
        PlayerSaveData.bHasSkillData = true;
        PlayerSaveData.LearnedSkills = SkillComponent->GetLearnedSkills();
        PlayerSaveData.SkillSlots = SkillComponent->GetSkillSlots();
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

bool UPTSaveSubsystem::ApplyPendingPlayerCharacterData(APTBasePlayerState* PlayerState)
{
    if (PlayerState == nullptr || !PlayerState->HasAuthority())
    {
        return false;
    }

    const FString PlayerSaveID = GetPlayerSaveID(PlayerState);
    FPTPlayerSaveData* PlayerSaveData = PendingPlayerCharacterData.Find(PlayerSaveID);
    if (PlayerSaveData == nullptr)
    {
        return false;
    }

    if (!ApplyToPlayerCharacter(PlayerState, *PlayerSaveData))
    {
        return false;
    }

    const int32 AppliedSaveVersion = PlayerSaveData->SaveVersion;
    PendingPlayerCharacterData.Remove(PlayerSaveID);
    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Save] Restored inventory, equipment, and skills. PlayerID=%s SaveVersion=%d"),
        *PlayerSaveID,
        AppliedSaveVersion);
    return true;
}

bool UPTSaveSubsystem::ApplyToPlayerCharacter(
    APTBasePlayerState* PlayerState,
    const FPTPlayerSaveData& PlayerSaveData) const
{
    AController* Controller = PlayerState != nullptr ? Cast<AController>(PlayerState->GetOwner()) : nullptr;
    APTPlayerCharacter* PlayerCharacter =
        Controller != nullptr ? Cast<APTPlayerCharacter>(Controller->GetPawn()) : nullptr;
    if (PlayerCharacter == nullptr || !PlayerCharacter->HasAuthority())
    {
        return false;
    }

    bool bAppliedAnyData = false;
    bool bAppliedAllData = true;

    if (PlayerSaveData.bHasInventoryData)
    {
        UPTInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent();
        bAppliedAnyData = true;
        bAppliedAllData &= InventoryComponent != nullptr &&
            InventoryComponent->RestoreInventorySlots(PlayerSaveData.InventorySlots);
    }

    if (PlayerSaveData.bHasEquipmentData)
    {
        UPTEquipmentComponent* EquipmentComponent = PlayerCharacter->GetEquipmentComponent();
        bAppliedAnyData = true;
        bAppliedAllData &= EquipmentComponent != nullptr &&
            EquipmentComponent->RestoreEquipmentSlots(PlayerSaveData.EquipmentSlots);
    }

    if (PlayerSaveData.bHasSkillData)
    {
        UPTPlayerSkillComponent* SkillComponent = PlayerCharacter->SkillComp;
        bAppliedAnyData = true;
        bAppliedAllData &= SkillComponent != nullptr &&
            SkillComponent->RestoreSkillProgress(PlayerSaveData.LearnedSkills, PlayerSaveData.SkillSlots);
    }

    return bAppliedAnyData && bAppliedAllData;
}

void UPTSaveSubsystem::MergeCharacterDataFromExistingSave(
    FPTPlayerSaveData& PlayerSaveData,
    const FPTPlayerSaveData& ExistingSaveData) const
{
    if (!PlayerSaveData.bHasInventoryData && ExistingSaveData.bHasInventoryData)
    {
        PlayerSaveData.bHasInventoryData = true;
        PlayerSaveData.InventorySlots = ExistingSaveData.InventorySlots;
    }

    if (!PlayerSaveData.bHasEquipmentData && ExistingSaveData.bHasEquipmentData)
    {
        PlayerSaveData.bHasEquipmentData = true;
        PlayerSaveData.EquipmentSlots = ExistingSaveData.EquipmentSlots;
    }

    if (!PlayerSaveData.bHasSkillData && ExistingSaveData.bHasSkillData)
    {
        PlayerSaveData.bHasSkillData = true;
        PlayerSaveData.LearnedSkills = ExistingSaveData.LearnedSkills;
        PlayerSaveData.SkillSlots = ExistingSaveData.SkillSlots;
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

void UPTSaveSubsystem::NotifyWorldReadyForAutoSave()
{
    StartAutoSave();
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
