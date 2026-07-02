// Fill out your copyright notice in the Description page of Project Settings.

#include "PTPartySlotWidget.h"
#include "PTPartyHealthBar.h"
#include "PTPartyManaBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTPartySlotWidget::NativeDestruct()
{
    // 정리
    ClearSlot();
    Super::NativeDestruct();
}

void UPTPartySlotWidget::SetupSlot(APTBasePlayerState* PS)
{
    if (!PS) return;
    BoundPS = PS;

    // 바 연결
    if (HealthBar) HealthBar->SetupPlayerState(PS);
    if (ManaBar)   ManaBar->SetupPlayerState(PS);

    // 레벨 연결
    PS->OnLevelChanged.AddUniqueDynamic(this, &UPTPartySlotWidget::HandleLevelChanged);

    // 이름 표시
    if (Txt_Name)
    {
        Txt_Name->SetText(FText::FromString(PS->GetPlayerName()));
    }

    // 초기 값 요청
    PS->BroadcastAllStats();
}

void UPTPartySlotWidget::ClearSlot()
{
    // 레벨 해제
    if (APTBasePlayerState* PS = BoundPS.Get())
    {
        PS->OnLevelChanged.RemoveDynamic(this, &UPTPartySlotWidget::HandleLevelChanged);
    }
    BoundPS = nullptr;
}

void UPTPartySlotWidget::HandleLevelChanged(int32 NewLevel)
{
    // 레벨 갱신
    if (Txt_Level)
    {
        Txt_Level->SetText(FText::AsNumber(NewLevel));
    }
}
