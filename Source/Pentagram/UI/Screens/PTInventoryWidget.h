// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTInventoryWidget.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // HUD 매니저가 이 위젯을 열 때 호출
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Widget")
    virtual void OpenWidget();

    // HUD 매니저가 이 위젯을 닫을 때 호출
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Widget")
    virtual void CloseWidget();

private:

    bool BShowInventoryWidget = false;
};
