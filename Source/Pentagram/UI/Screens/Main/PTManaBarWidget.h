// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "PTManaBarWidget.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTManaBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void HandleManaChanged(float Current, float Max);

protected:
    virtual void NativeConstruct() override;
};
