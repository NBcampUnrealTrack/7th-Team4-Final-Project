#include "PTDeathMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Character/Player/PTPlayerController.h"

void UPTDeathMenuWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    if (Btn_Respawn)
    {
        // 재활성화 중복 바인딩 방지
        Btn_Respawn->OnClicked.AddUniqueDynamic(this, &UPTDeathMenuWidget::OnRespawnClicked);
        Btn_Respawn->SetStyle(LockedButtonStyle); // 대기 이미지
        Btn_Respawn->SetIsEnabled(false);         // 처음엔 잠금
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
    // 바인딩 해제
    if (Btn_Respawn)
    {
        Btn_Respawn->OnClicked.RemoveDynamic(this, &UPTDeathMenuWidget::OnRespawnClicked);
    }

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
                FString::Printf(TEXT("Respawn in %d"), RemainingSeconds)));
        }
        --RemainingSeconds;
        return;
    }

    // 대기 종료
    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

    if (Text_Countdown)
    {
        Text_Countdown->SetText(FText::FromString(TEXT("Click to Respawn")));
    }

    if (Btn_Respawn)
    {
        Btn_Respawn->SetStyle(ReadyButtonStyle); // 활성 이미지
        Btn_Respawn->SetIsEnabled(true);
        Btn_Respawn->SetFocus();
    }
}

void UPTDeathMenuWidget::OnRespawnClicked()
{
    // 부활 요청
    if (APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PC->Server_RequestRespawn();
    }

    DeactivateWidget();
}
