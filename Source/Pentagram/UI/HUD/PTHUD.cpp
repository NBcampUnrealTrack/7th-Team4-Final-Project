// Fill out your copyright notice in the Description page of Project Settings.


#include "PTHUD.h"
#include "UI/Widget/PTPrimaryLayout.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Widget/PTActivatableWidgetBase.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

void APTHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    ULocalPlayer* LP = PC->GetLocalPlayer();
    if (!LP) return;

    // 1) PrimaryLayout 생성 & 화면 추가
    if (MainLayoutClass)
    {
        MainLayoutWidget = CreateWidget<UPTPrimaryLayout>(PC, MainLayoutClass);
        if (MainLayoutWidget)
        {
            MainLayoutWidget->AddToPlayerScreen(0);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[PTUI] APTHUD: CreateWidget(MainLayoutClass) failed"));
            return;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTUI] APTHUD: MainLayoutClass not set in BP"));
        return;
    }

    // 2) 매니저에 PrimaryLayout 등록
    UPTUIManagerSubsystem* UIManager = LP->GetSubsystem<UPTUIManagerSubsystem>();
    if (!UIManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTUI] APTHUD: UIManagerSubsystem not found"));
        return;
    }
    UIManager->RegisterPrimaryLayout(MainLayoutWidget);

    // 3) 초기 HUD 위젯이 지정되어 있으면 GameLayer에 푸시
    if (InitialHUDWidgetClass)
    {
        UIManager->PushWidget(InitialHUDWidgetClass, EPTUILayer::HUD);
    }
}

void APTHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (MainLayoutWidget)
    {
        MainLayoutWidget->RemoveFromParent();
        MainLayoutWidget = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}
