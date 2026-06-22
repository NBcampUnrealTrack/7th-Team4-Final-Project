// Fill out your copyright notice in the Description page of Project Settings.

#include "PTStatBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/Player/PTBasePlayerState.h"

void UPTStatBarWidget::NativeDestruct()
{
    if (APTBasePlayerState* PS = BoundPS.Get())
    {
        UnbindFromPlayerState(PS);
    }
    BoundPS = nullptr;
    Super::NativeDestruct();

}

void UPTStatBarWidget::SetValue(float Current, float Max)
{
    // 목표치 설정
    MaxValue      = FMath::Max(Max, KINDA_SMALL_NUMBER);
    TargetCurrent = FMath::Clamp(Current, 0.f, MaxValue);
}

void UPTStatBarWidget::SetValueInstant(float Current, float Max)
{
    // 즉시 적용
    SetValue(Current, Max);
    DisplayCurrent = TargetCurrent;
    ApplyDisplay();
}

void UPTStatBarWidget::SetupPlayerState(APTBasePlayerState* PS)
{
    if (!PS || BoundPS.Get() == PS) return;

    if (APTBasePlayerState* Old = BoundPS.Get())
    {
        UnbindFromPlayerState(Old);
    }
    BoundPS = PS;
    BindToPlayerState(PS);
}

void UPTStatBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 반영
    DisplayCurrent = TargetCurrent;
    ApplyDisplay();
}

void UPTStatBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 도달 시 종료
    if (FMath::IsNearlyEqual(DisplayCurrent, TargetCurrent, 0.01f))
    {
        if (DisplayCurrent != TargetCurrent)
        {
            DisplayCurrent = TargetCurrent;
            ApplyDisplay();
        }
        return;
    }

    // 값 보간
    if (InterpSpeed > 0.f)
    {
        DisplayCurrent = FMath::FInterpTo(DisplayCurrent, TargetCurrent, InDeltaTime, InterpSpeed);
    }
    else
    {
        DisplayCurrent = TargetCurrent;
    }

    // 화면 표시
    ApplyDisplay();
}

void UPTStatBarWidget::ApplyDisplay()
{
    // 비율 계산
    const float Percent = (MaxValue > 0.f) ? (DisplayCurrent / MaxValue) : 0.f;

    // 바 갱신
    if (PB_Bar)
    {
        PB_Bar->SetPercent(Percent);
    }

    // 텍스트 갱신
    if (Txt_Value)
    {
        const FString Str = FString::Printf(TEXT("%d / %d"),
            FMath::RoundToInt(DisplayCurrent),
            FMath::RoundToInt(MaxValue));
        Txt_Value->SetText(FText::FromString(Str));
    }

    // BP 이벤트
    OnDisplayValueUpdated(DisplayCurrent, MaxValue, Percent);
}
