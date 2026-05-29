#include "Character/Player/PTBasePlayerState.h"

#include "Net/UnrealNetwork.h"

void APTBasePlayerState::OnRep_CurrentHP()
{
    // HP 변경 시 UI 델리게이트 발행 (UI연동 필요)
}

void APTBasePlayerState::OnRep_CurrentMP()
{
    // MP 변경 시 UI 델리게이트 발행 (UI연동 필요)
}

void APTBasePlayerState::OnRep_RequiredExp()
{
    //  시 UI 델리게이트 발행 (UI연동 필요)
}

void APTBasePlayerState::OnRep_PlayerLevel()
{
    //  시 UI 델리게이트 발행 (UI연동 필요)
}

void APTBasePlayerState::OnRep_CurrentGold()
{
    // 시 UI 델리게이트 발행 (UI연동 필요)
}

void APTBasePlayerState::OnRep_CurrentExp()
{
    // UI 델리게이트 발행 (UI연동 필요)
}

void APTBasePlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTBasePlayerState, CurrentHP);
    DOREPLIFETIME(APTBasePlayerState, MaxHP);
    DOREPLIFETIME(APTBasePlayerState, CurrentMP);
    DOREPLIFETIME(APTBasePlayerState, MaxMP);
    DOREPLIFETIME(APTBasePlayerState, CurrentGold);
    DOREPLIFETIME(APTBasePlayerState, CurrentExp);
    DOREPLIFETIME(APTBasePlayerState, PlayerLevel);
    DOREPLIFETIME(APTBasePlayerState, RequiredExp);
}
