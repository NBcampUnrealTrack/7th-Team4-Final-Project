// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/PTItemTypes.h"
#include "PTItemTooltipWidget.generated.h"

class UTextBlock;

// 아이템 설명
UCLASS()
class PENTAGRAM_API UPTItemTooltipWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // 슬롯 표시
    UFUNCTION(BlueprintCallable, Category = "PT|Tooltip")
    void SetItem(const FInventorySlot& InSlot);

    // 비우기
    UFUNCTION(BlueprintCallable, Category = "PT|Tooltip")
    void Clear();

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Name;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Grade;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Type;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_BaseStat;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Options;

private:
    static FString EnumToString(const UEnum* EnumPtr, int64 Value);
};
