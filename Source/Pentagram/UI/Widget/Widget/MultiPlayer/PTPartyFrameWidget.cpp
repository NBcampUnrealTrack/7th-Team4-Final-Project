// Fill out your copyright notice in the Description page of Project Settings.

#include "PTPartyFrameWidget.h"
#include "PTPartySlotWidget.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UPTPartyFrameWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 즉시 1회
    RefreshParty();

    // 주기 갱신
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RefreshTimerHandle, this, &UPTPartyFrameWidget::RefreshParty, RefreshInterval, true);
    }
}

void UPTPartyFrameWidget::NativeDestruct()
{
    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshTimerHandle);
    }
    Super::NativeDestruct();
}

void UPTPartyFrameWidget::RefreshParty()
{
    if (!Box_Slots || !SlotWidgetClass) return;

    UWorld* World = GetWorld();
    AGameStateBase* GS = World ? World->GetGameState() : nullptr;
    if (!GS) return;

    // 본인 PS
    APlayerState* LocalPS = nullptr;
    if (APlayerController* PC = GetOwningPlayer())
    {
        LocalPS = PC->PlayerState;
    }

    // 현재 파티원
    TSet<APTBasePlayerState*> CurrentMembers;
    for (APlayerState* PS : GS->PlayerArray)
    {
        APTBasePlayerState* PTPS = Cast<APTBasePlayerState>(PS);
        if (!PTPS) continue;
        if (!bIncludeLocalPlayer && PTPS == LocalPS) continue;
        CurrentMembers.Add(PTPS);
    }

    // 떠난 인원 제거
    for (auto It = SlotMap.CreateIterator(); It; ++It)
    {
        APTBasePlayerState* Key = It->Key.Get();
        if (!Key || !CurrentMembers.Contains(Key))
        {
            if (It->Value)
            {
                It->Value->ClearSlot();
                It->Value->RemoveFromParent();
            }
            It.RemoveCurrent();
        }
    }

    // 새 인원 추가
    for (APTBasePlayerState* PTPS : CurrentMembers)
    {
        if (SlotMap.Contains(PTPS)) continue;

        UPTPartySlotWidget* PartySlot = CreateWidget<UPTPartySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
        if (!PartySlot) continue;

        // 붙이기 (한 번만)
        if (UPanelSlot* PanelSlot = Box_Slots->AddChild(PartySlot))
        {
            if (UVerticalBoxSlot* VBSlot = Cast<UVerticalBoxSlot>(PanelSlot))
            {
                VBSlot->SetPadding(FMargin(0.f, 0.f, 0.f, SlotSpacing));
            }
        }

        // 붙인 뒤 세팅
        PartySlot->SetupSlot(PTPS);

        SlotMap.Add(PTPS, PartySlot);
    }
}
