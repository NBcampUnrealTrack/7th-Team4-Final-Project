// Fill out your copyright notice in the Description page of Project Settings.

#include "PTStatBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPTStatBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기화 및 즉시 UI 반영
    DisplayCurrent = TargetCurrent;
    ApplyDisplay();
}

void UPTStatBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 목표값에 도달했으면 종료
    if (FMath::IsNearlyEqual(DisplayCurrent, TargetCurrent, 0.01f))
    {
        return;
    }

    // 목표값을 향해 부드럽게 수치 변경 (보간)
    if (InterpSpeed > 0.f)
    {
        DisplayCurrent = FMath::FInterpTo(DisplayCurrent, TargetCurrent, InDeltaTime, InterpSpeed);
    }
    else
    {
        DisplayCurrent = TargetCurrent;
    }

    // 변경된 수치 화면에 표시
    ApplyDisplay();
}

void UPTStatBarWidget::SetValue(float Current, float Max)
{
    // 새 목표치 설정 (애니메이션 O)
    MaxValue      = FMath::Max(Max, KINDA_SMALL_NUMBER);
    TargetCurrent = FMath::Clamp(Current, 0.f, MaxValue);
}

void UPTStatBarWidget::SetValueInstant(float Current, float Max)
{
    // 애니메이션 없이 즉시 값 변경 (애니메이션 X)
    SetValue(Current, Max);
    DisplayCurrent = TargetCurrent;
    ApplyDisplay();
}

void UPTStatBarWidget::ApplyDisplay()
{
    // 게이지 비율 계산
    const float Percent = (MaxValue > 0.f) ? (DisplayCurrent / MaxValue) : 0.f;

    // 프로그레스 바 갱신
    if (PB_Bar)
    {
        PB_Bar->SetPercent(Percent);
    }

    // 텍스트(예: 120 / 200) 갱신
    if (Txt_Value)
    {
        const FString Str = FString::Printf(TEXT("%d / %d"),
            FMath::FloorToInt(DisplayCurrent),
            FMath::FloorToInt(MaxValue));
        Txt_Value->SetText(FText::FromString(Str));
    }

    // 블루프린트 이벤트 호출
    OnDisplayValueUpdated(DisplayCurrent, MaxValue, Percent);
}
