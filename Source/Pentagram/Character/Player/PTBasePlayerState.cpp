#include "Character/Player/PTBasePlayerState.h"

#include "Core/Subsystems/PTQuestSubsystem.h"
#include "UI/Data/PTDelegates.h"
#include "Net/UnrealNetwork.h"
#include "Core/PTGameMode.h"     // 로비 추가 (경로는 프로젝트에 맞게)
#include "Core/PTGameState.h"    // 로비 추가 (경로는 프로젝트에 맞게)

void APTBasePlayerState::OnRep_CurrentHP()
{
    OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void APTBasePlayerState::OnRep_MaxHP()
{
    OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void APTBasePlayerState::OnRep_CurrentMP()
{
    OnManaChanged.Broadcast(CurrentMP, MaxMP);
}

void APTBasePlayerState::OnRep_MaxMP()
{
    OnManaChanged.Broadcast(CurrentMP, MaxMP);
}

void APTBasePlayerState::OnRep_BaseAtk()
{
    OnAttackChanged.Broadcast(BaseAtk);
}

void APTBasePlayerState::OnRep_BaseDef()
{
    OnDefenseChanged.Broadcast(BaseDef);
}

void APTBasePlayerState::OnRep_CriticalChance()
{
    OnCriticalChanged.Broadcast(CriticalChance, CriticalATK);
}

void APTBasePlayerState::OnRep_CriticalATK()
{
    OnCriticalChanged.Broadcast(CriticalChance, CriticalATK);
}

void APTBasePlayerState::OnRep_MoveSpeed()
{
    OnMoveSpeedChanged.Broadcast(MoveSpeed);
}

void APTBasePlayerState::OnRep_CurrentGold()
{
    OnGoldChanged.Broadcast(CurrentGold);
}

void APTBasePlayerState::OnRep_CurrentExp()
{
    OnExpChanged.Broadcast(CurrentExp, RequiredExp);
}

void APTBasePlayerState::OnRep_RequiredExp()
{
    OnExpChanged.Broadcast(CurrentExp, RequiredExp);
}

void APTBasePlayerState::OnRep_PlayerLevel()
{
    OnLevelChanged.Broadcast(PlayerLevel);
}

// 로비 추가
void APTBasePlayerState::OnRep_IsReady()
{
    UWorld* World = GetWorld();
    if (APTGameState* PTGameState = World != nullptr ? World->GetGameState<APTGameState>() : nullptr)
    {
        PTGameState->NotifyLobbyUpdated();
    }
}
void APTBasePlayerState::OnRep_AcceptedQuests()
{
    UGameInstance* GameInstance = GetGameInstance();
    UPTQuestSubsystem* QuestSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTQuestSubsystem>() : nullptr;
    if (QuestSubsystem != nullptr)
    {
        QuestSubsystem->BroadcastQuestListChanged();
    }
}

void APTBasePlayerState::BroadcastAllStats()
{
    OnHealthChanged.Broadcast(CurrentHP, MaxHP);
    OnManaChanged.Broadcast(CurrentMP, MaxMP);
    OnAttackChanged.Broadcast(BaseAtk);
    OnDefenseChanged.Broadcast(BaseDef);
    OnCriticalChanged.Broadcast(CriticalChance, CriticalATK);
    OnMoveSpeedChanged.Broadcast(MoveSpeed);
    OnLevelChanged.Broadcast(PlayerLevel);
    OnExpChanged.Broadcast(CurrentExp, RequiredExp);
    OnGoldChanged.Broadcast(CurrentGold);
}

// 리스폰 위치를 장부에 기록
void APTBasePlayerState::SetSavedRespawnLocation(const FVector& NewLocation)
{
    SavedRespawnLocation = NewLocation;
    bHasRespawnLocation = true;
}

// 로비 추가
void APTBasePlayerState::SetReady(bool bNewReady)
{
    if (!HasAuthority())
    {
        return;
    }
    if (bIsReady == bNewReady)
    {
        return;
    }

    bIsReady = bNewReady;
    OnRep_IsReady();    // 서버 반영

    UWorld* World = GetWorld();
    if (APTGameMode* PTGameMode = World != nullptr ? World->GetAuthGameMode<APTGameMode>() : nullptr)
    {
        PTGameMode->NotifyReadyChanged();
    }
}

void APTBasePlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTBasePlayerState, CurrentHP);
    DOREPLIFETIME(APTBasePlayerState, MaxHP);
    DOREPLIFETIME(APTBasePlayerState, CurrentMP);
    DOREPLIFETIME(APTBasePlayerState, MaxMP);
    DOREPLIFETIME(APTBasePlayerState, BaseAtk);
    DOREPLIFETIME(APTBasePlayerState, BaseDef);
    DOREPLIFETIME(APTBasePlayerState, CriticalChance);
    DOREPLIFETIME(APTBasePlayerState, CriticalATK);
    DOREPLIFETIME(APTBasePlayerState, MoveSpeed);
    DOREPLIFETIME(APTBasePlayerState, CurrentGold);
    DOREPLIFETIME(APTBasePlayerState, CurrentExp);
    DOREPLIFETIME(APTBasePlayerState, PlayerLevel);
    DOREPLIFETIME(APTBasePlayerState, RequiredExp);
    DOREPLIFETIME(APTBasePlayerState, bIsReady);    // 로비 추가
    DOREPLIFETIME(APTBasePlayerState, AcceptedQuests);
}
