// Fill out your copyright notice in the Description page of Project Settings.

#include "PTExpBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Engine/LocalPlayer.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Widget/Notify/PTNotifyTypes.h" // 실제 경로에 맞게 수정

void UPTExpBarWidget::HandleExpChanged(float Current, float Required)
{
    SetValue(Current, Required);
}

void UPTExpBarWidget::HandleLevelChanged(int32 NewLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("[Notify][1] HandleLevelChanged called. NewLevel=%d LastKnownLevel=%d"),
        NewLevel, LastKnownLevel);

    // 레벨 텍스트는 항상 최신값으로 갱신 (초기 동기화든 실제 레벨업이든)
    UpdateLevelText(NewLevel);

    // 최초 동기화(BindToPlayerState 직후 BroadcastAllStats 등)인지,
    // 진짜 레벨이 오른 건지 구분
    const bool bIsInitialSync = (LastKnownLevel < 0);
    const bool bIsRealLevelUp = !bIsInitialSync && NewLevel > LastKnownLevel;

    LastKnownLevel = NewLevel;

    if (!bIsRealLevelUp)
    {
        // 최초 동기화거나, 레벨이 오히려 같거나 낮게 온 경우(리스폰 등) -> 알림/연출 스킵
        return;
    }

    // 레벨업 초기화 연출은 진짜 레벨업일 때만
    SetValueInstant(0.f, MaxValue);
    OnLevelUpVisual(NewLevel);

    // 상단 알림 위젯으로 레벨업 표시
    ShowLevelUpNotify(NewLevel);
}

void UPTExpBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PB_Bar)
    {
        PB_Bar->SetBarFillType(EProgressBarFillType::LeftToRight);
    }
}

void UPTExpBarWidget::BindToPlayerState(APTBasePlayerState* PS)
{
    PS->OnExpChanged.AddUniqueDynamic(this, &UPTExpBarWidget::HandleExpChanged);
    PS->OnLevelChanged.AddUniqueDynamic(this, &UPTExpBarWidget::HandleLevelChanged);
}

void UPTExpBarWidget::UnbindFromPlayerState(APTBasePlayerState* PS)
{
    PS->OnExpChanged.RemoveDynamic(this, &UPTExpBarWidget::HandleExpChanged);
    PS->OnLevelChanged.RemoveDynamic(this, &UPTExpBarWidget::HandleLevelChanged);
}

void UPTExpBarWidget::UpdateLevelText(int32 NewLevel)
{
    if (Txt_Level)
    {
        Txt_Level->SetText(FText::FromString(FString::Printf(TEXT("Lv %d"), NewLevel)));
    }
}

void UPTExpBarWidget::ShowLevelUpNotify(int32 NewLevel)
{
    ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("[Notify][2] FAILED - GetOwningLocalPlayer() returned null"));
        return;
    }

    UPTUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UIManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[Notify][2] FAILED - GetSubsystem<UPTUIManagerSubsystem>() returned null"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Notify][2] UIManager OK, calling ShowNotify"));

    FPTNotifyData Data;
    Data.Type = EPTNotifyType::LevelUp;
    Data.Message = FText::Format(
        NSLOCTEXT("PTExpBarWidget", "LevelUpFormat", "Level Up! Lv.{0}"),
        FText::AsNumber(NewLevel));
    Data.Duration = 3.0f;

    UIManager->ShowNotify(Data);
}
