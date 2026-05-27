// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "PTExpBarWidget.generated.h"

/**
 *
 */
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

    /** 레벨업 시 BP에서 폴리시/이펙트 처리 */
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|StatBar")
    void OnLevelUpVisual(int32 NewLevel);
};
