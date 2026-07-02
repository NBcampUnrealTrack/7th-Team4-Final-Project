// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/Widget/Player/PTStatBarWidget.h"
#include "PTPartyManaBar.generated.h"

class APTBasePlayerState;

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTPartyManaBar : public UPTStatBarWidget
{
    GENERATED_BODY()

protected:
    // 바인드
    virtual void BindToPlayerState(APTBasePlayerState* PS) override;
    virtual void UnbindFromPlayerState(APTBasePlayerState* PS) override;

    // 마나 변경
    UFUNCTION()
    void HandleManaChanged(float CurrentMP, float MaxMP);

private:
    // 첫 수신
    bool bReceivedFirst = false;
};
