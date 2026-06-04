// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTHealthBarwidget.generated.h"

/**
 *
 */
class APTBasePlayerState;
UCLASS()
class PENTAGRAM_API UPTHealthBarwidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
   //델리게이트 변수 선언
    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

protected:
    virtual void NativeConstruct() override;

    virtual void BindToPlayerState(APTBasePlayerState* PS) override;
    virtual void UnbindFromPlayerState(APTBasePlayerState* PS) override;
};
