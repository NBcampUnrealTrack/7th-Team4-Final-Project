// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTDeathMenuWidget.generated.h"

class UButton;     // 변경: UButton
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTDeathMenuWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Restart;   // 변경: UButton

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_MainMenu;  // 변경: UButton

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Countdown; // 카운트 표시

    UPROPERTY(EditDefaultsOnly, Category = "Respawn")
    float RespawnDelay = 5.0f; // 대기 시간

private:
    void UpdateCountdown(); // 카운트 갱신

    UFUNCTION()
    void OnRestartClicked();

    UFUNCTION()
    void OnMainMenuClicked();

    FTimerHandle CountdownTimerHandle;
    int32 RemainingSeconds = 0;
};
