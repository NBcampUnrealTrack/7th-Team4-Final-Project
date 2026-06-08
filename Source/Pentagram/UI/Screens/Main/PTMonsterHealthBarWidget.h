// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTMonsterHealthBarWidget.generated.h"

/**
 *
 */
class APTMonsterCharacter;
UCLASS()
class PENTAGRAM_API UPTMonsterHealthBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:

    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

protected:
    virtual void NativeConstruct() override;

    virtual void BindToPlayerState(APTBasePlayerState* PS) override;
    virtual void UnbindFromPlayerState(APTBasePlayerState* PS) override;
};
