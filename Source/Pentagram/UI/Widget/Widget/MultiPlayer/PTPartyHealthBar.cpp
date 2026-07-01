// Fill out your copyright notice in the Description page of Project Settings.

#include "PTPartyHealthBar.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTPartyHealthBar::BindToPlayerState(APTBasePlayerState* PS)
{
    if (!PS) return;

    // 구독
    PS->OnHealthChanged.AddUniqueDynamic(this, &UPTPartyHealthBar::HandleHealthChanged);

    // 현재 값 즉시 반영 (MaxHP 유효할 때만)
    if (PS->MaxHP > 0.f)
    {
        bReceivedFirst = true;
        SetValueInstant(PS->CurrentHP, PS->MaxHP);
    }
}

void UPTPartyHealthBar::UnbindFromPlayerState(APTBasePlayerState* PS)
{
    if (!PS) return;

    // 등록 해제
    PS->OnHealthChanged.RemoveDynamic(this, &UPTPartyHealthBar::HandleHealthChanged);
}

void UPTPartyHealthBar::HandleHealthChanged(float CurrentHP, float MaxHP)
{
    // 첫 값 즉시
    if (!bReceivedFirst)
    {
        bReceivedFirst = true;
        SetValueInstant(CurrentHP, MaxHP);
        return;
    }

    // 이후 보간
    SetValue(CurrentHP, MaxHP);
}
