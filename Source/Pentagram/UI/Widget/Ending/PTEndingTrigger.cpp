// Fill out your copyright notice in the Description page of Project Settings.

#include "PTEndingTrigger.h"
#include "CommonActivatableWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "UI/Manage/PTUIManagerSubsystem.h"

APTEndingTrigger::APTEndingTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APTEndingTrigger::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(
        EndingTimerHandle,
        this,
        &APTEndingTrigger::ShowEndingWidget,
        DelaySeconds,
        false);
}

void APTEndingTrigger::ShowEndingWidget()
{
    if (!EndingWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ending] EndingWidgetClass가 비어있음"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    if (UPTUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPTUIManagerSubsystem>())
    {
        UIManager->PushWidget(EndingWidgetClass, EPTUILayer::Modal);
    }
}
