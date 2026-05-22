// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "PTPrimaryLayout.generated.h"

class UCommonActivatableWidgetStack;

UCLASS()
class PENTAGRAM_API UPTPrimaryLayout : public UCommonUserWidget
{
	GENERATED_BODY()


public:
    //레이어 enum에 대응하는 스택 반환.
    UCommonActivatableWidgetStack* GetLayerStack(EPTUILayer Layer) const;

protected:
    // 1. 항상 떠 있는 UI층 (HP바, 미니맵, 퀵슬롯 등)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    UCommonActivatableWidgetStack* GameLayer;

    // 2. 열고 닫는 메뉴층 (인벤토리, 스킬창, 일시정지 메뉴 등)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    UCommonActivatableWidgetStack* MenuLayer;

    // 3. 최상단 경고/안내 팝업층 ("정말 종료하시겠습니까?" 등)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    UCommonActivatableWidgetStack* ModalLayer;
};
