#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Styling/SlateTypes.h"
#include "PTDeathMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTDeathMenuWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Respawn;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Countdown;

    UPROPERTY(EditDefaultsOnly, Category = "Respawn")
    float RespawnDelay = 5.0f;

    // 대기 중 버튼 이미지
    UPROPERTY(EditDefaultsOnly, Category = "Respawn")
    FButtonStyle LockedButtonStyle;

    // 활성 버튼 이미지
    UPROPERTY(EditDefaultsOnly, Category = "Respawn")
    FButtonStyle ReadyButtonStyle;

private:
    void UpdateCountdown();

    UFUNCTION()
    void OnRespawnClicked();

    FTimerHandle CountdownTimerHandle;
    int32 RemainingSeconds = 0;
};
