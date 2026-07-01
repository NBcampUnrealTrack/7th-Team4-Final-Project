// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/Widget/Player/PTStatBarWidget.h"
#include "PTPartyHealthBar.generated.h"

class APTBasePlayerState;

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTPartyHealthBar : public UPTStatBarWidget
{
    GENERATED_BODY()

protected:
    // 바인드
    virtual void BindToPlayerState(APTBasePlayerState* PS) override;
    virtual void UnbindFromPlayerState(APTBasePlayerState* PS) override;

    // 체력 변경
    UFUNCTION()
    void HandleHealthChanged(float CurrentHP, float MaxHP);

private:
    // 첫 수신
    bool bReceivedFirst = false;
};
