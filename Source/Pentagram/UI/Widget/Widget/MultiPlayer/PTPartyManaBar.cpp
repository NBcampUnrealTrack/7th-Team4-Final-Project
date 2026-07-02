// Fill out your copyright notice in the Description page of Project Settings.

#include "PTPartyManaBar.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTPartyManaBar::BindToPlayerState(APTBasePlayerState* PS)
{
    if (!PS) return;

    PS->OnManaChanged.AddUniqueDynamic(this, &UPTPartyManaBar::HandleManaChanged);

    if (PS->MaxMP > 0.f)
    {
        bReceivedFirst = true;
        SetValueInstant(PS->CurrentMP, PS->MaxMP);
    }
}

void UPTPartyManaBar::UnbindFromPlayerState(APTBasePlayerState* PS)
{
    if (!PS) return;

    // 등록 해제
    PS->OnManaChanged.RemoveDynamic(this, &UPTPartyManaBar::HandleManaChanged);
}

void UPTPartyManaBar::HandleManaChanged(float CurrentMP, float MaxMP)
{
    // 첫 값 즉시
    if (!bReceivedFirst)
    {
        bReceivedFirst = true;
        SetValueInstant(CurrentMP, MaxMP);
        return;
    }

    // 이후 보간
    SetValue(CurrentMP, MaxMP);
}
