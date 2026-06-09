// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTManaBarWidget.generated.h"

class APTBasePlayerState;

UCLASS()
class PENTAGRAM_API UPTManaBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    // MP 콜백
    UFUNCTION()
    void HandleManaChanged(float Current, float Max);

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void BindToPlayerState(APTBasePlayerState* PS) override;
    virtual void UnbindFromPlayerState(APTBasePlayerState* PS) override;
};
