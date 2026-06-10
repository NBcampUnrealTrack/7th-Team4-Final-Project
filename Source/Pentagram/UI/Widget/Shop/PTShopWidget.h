// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTShopWidget.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTShopWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    // 오버라이드
    virtual void NativeOnInitialized() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;
};
