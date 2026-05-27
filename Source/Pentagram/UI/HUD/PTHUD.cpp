// Fill out your copyright notice in the Description page of Project Settings.

#include "PTHUD.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "UI/Screens/PTHUDWidget.h"
#include "UI/Screens/LayOut/PTPrimaryLayout.h"

void APTHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    ULocalPlayer* LP = PC->GetLocalPlayer();
    if (!LP || !MainLayoutClass) return;

    // 1. 베이스 레이아웃 생성 및 화면 추가
    MainLayoutWidget = CreateWidget<UPTPrimaryLayout>(PC, MainLayoutClass);
    if (!MainLayoutWidget) return;

    MainLayoutWidget->AddToPlayerScreen(0);

    // 2. UI 매니저에 레이아웃 등록
    UPTUIManagerSubsystem* UIManager = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UIManager) return;

    UIManager->RegisterPrimaryLayout(MainLayoutWidget);

    // 3. 기본 HUD 위젯 출력
    if (InitialHUDWidgetClass)
    {
        UIManager->PushWidget(InitialHUDWidgetClass, EPTUILayer::HUD);
    }
}

void APTHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 레이아웃 제거 및 초기화
    if (MainLayoutWidget)
    {
        MainLayoutWidget->RemoveFromParent();
        MainLayoutWidget = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}
