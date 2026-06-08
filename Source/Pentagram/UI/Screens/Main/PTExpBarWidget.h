// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTExpBarWidget.generated.h"

/**
 *
 */
class APTBasePlayerState;

UCLASS()
class PENTAGRAM_API UPTExpBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void HandleExpChanged(float Current, float Required);

    UFUNCTION()
    void HandleLevelChanged(int32 NewLevel);

protected:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|StatBar")
    void OnLevelUpVisual(int32 NewLevel);

    virtual void BindToPlayerState(APTBasePlayerState* PS) override;
    virtual void UnbindFromPlayerState(APTBasePlayerState* PS) override;
};
