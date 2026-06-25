// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTLobbySlotWidget.generated.h"

/**
 *
 */
class UTextBlock;
class UImage;

UCLASS()
class PENTAGRAM_API UPTLobbySlotWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:

    void SetSlot(const FString& InName, int32 InLevel, bool bInReady);
    void SetEmpty();

protected:

    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Img_Background;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> SlotNumberText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerLevelText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ReadyStateText;

    UPROPERTY(EditAnywhere, Category = "Slot")
    int32 SlotNumber = 1;

    UPROPERTY(EditAnywhere, Category = "Slot")
    TObjectPtr<UTexture2D> EmptyBackground;

    UPROPERTY(EditAnywhere, Category = "Slot")
    TObjectPtr<UTexture2D> FilledBackground;
};
