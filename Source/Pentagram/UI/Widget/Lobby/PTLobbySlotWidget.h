// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTLobbySlotWidget.generated.h"

/**
 *
 */
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTLobbySlotWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:

    void SetSlot(const FString& InName, int32 InLevel, bool bInReady);
    void SetEmpty();

protected:

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerLevelText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ReadyStateText;
};
