// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "PTHealthBarwidget.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTHealthBarwidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    /** PTStatComponent의 FPTOnHealthChanged 델리게이트가 직접 바인딩 가능한 시그니처 */
    UFUNCTION()
    void HandleHealthChanged(float Current, float Max);

protected:
    virtual void NativeConstruct() override;
};
