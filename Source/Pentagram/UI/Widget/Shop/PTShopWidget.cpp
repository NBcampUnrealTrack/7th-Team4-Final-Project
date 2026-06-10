// Fill out your copyright notice in the Description page of Project Settings.


#include "PTShopWidget.h"

void UPTShopWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UPTShopWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
}

void UPTShopWidget::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
}

bool UPTShopWidget::NativeOnHandleBackAction()
{
    bIsBackHandler = true;
    DeactivateWidget(); // 닫기
    return true;
}
