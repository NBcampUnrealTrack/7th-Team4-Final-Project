// Fill out your copyright notice in the Description page of Project Settings.


#include "PTDeathMenuWidget.h"
#include "Components/Button.h"    // 변경: UButton
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UPTDeathMenuWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    if (Btn_Restart)
    {
        // 변경: UButton 바인딩
        Btn_Restart->OnClicked.AddDynamic(this, &UPTDeathMenuWidget::OnRestartClicked);
        Btn_Restart->SetIsEnabled(false); // 처음엔 잠금
    }

    if (Btn_MainMenu)
    {
        // 변경: UButton 바인딩
        Btn_MainMenu->OnClicked.AddDynamic(this, &UPTDeathMenuWidget::OnMainMenuClicked);
    }

    // 대기 시작
    RemainingSeconds = FMath::CeilToInt(RespawnDelay);
    UpdateCountdown();

    GetWorld()->GetTimerManager().SetTimer(
        CountdownTimerHandle,
        this,
        &UPTDeathMenuWidget::UpdateCountdown,
        1.0f,
        true);
}

void UPTDeathMenuWidget::NativeOnDeactivated()
{
    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CountdownTimerHandle);
    }

    Super::NativeOnDeactivated();
}

void UPTDeathMenuWidget::UpdateCountdown()
{
    if (RemainingSeconds > 0)
    {
        if (Text_Countdown)
        {
            Text_Countdown->SetText(FText::FromString(
                FString::Printf(TEXT("%d초 후 부활"), RemainingSeconds)));
        }
        --RemainingSeconds;
        return;
    }

    // 대기 종료
    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

    if (Text_Countdown)
    {
        Text_Countdown->SetText(FText::FromString(TEXT("부활 가능")));
    }

    if (Btn_Restart)
    {
        Btn_Restart->SetIsEnabled(true);
        Btn_Restart->SetFocus();
    }
}

void UPTDeathMenuWidget::OnRestartClicked()
{
    // 위젯 닫기
    DeactivateWidget();

    // 현재 레벨 재시작
    FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
    UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevelName));
}

void UPTDeathMenuWidget::OnMainMenuClicked()
{
    DeactivateWidget();

    // 메인 메뉴 이동
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}
