// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/PTHUDWidget.h"

#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/Button.h"
#include "Core/Subsystems/PTOnlineSubsystem.h"
#include "Engine/GameInstance.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/Widget/Widget/Monster/PTMonsterHealthBarWidget.h"

UPTHUDWidget::UPTHUDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bAutoActivate  = true;
    bIsBackHandler = false;
}

void UPTHUDWidget::HandleMonsterTargeted(AActor* TargetMonster)
{
    if (!MonsterTargetFrame)
    {
        return;
    }

    if (APTMonsterCharacter* Monster = Cast<APTMonsterCharacter>(TargetMonster))
    {
        MonsterTargetFrame->ActivateForMonster(Monster);
    }
}


void UPTHUDWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer());

    if (PC)
    {
        PC->OnMonsterTargeted.AddUniqueDynamic(this, &UPTHUDWidget::HandleMonsterTargeted);
    }

    if (InviteButton && !InviteButton->OnClicked.IsAlreadyBound(this, &UPTHUDWidget::OpenSteamInviteUI))
    {
        InviteButton->OnClicked.AddDynamic(this, &UPTHUDWidget::OpenSteamInviteUI);
    }
}


void UPTHUDWidget::NativeOnDeactivated()
{
    if (APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PC->OnMonsterTargeted.RemoveDynamic(this, &UPTHUDWidget::HandleMonsterTargeted);
    }

    if (InviteButton)
    {
        InviteButton->OnClicked.RemoveDynamic(this, &UPTHUDWidget::OpenSteamInviteUI);
    }

    Super::NativeOnDeactivated();
}

bool UPTHUDWidget::NativeOnHandleBackAction()
{
    return false;
}

void UPTHUDWidget::OpenSteamInviteUI()
{
    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
    UPTOnlineSubsystem* OnlineSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTOnlineSubsystem>() : nullptr;
    if (OnlineSubsystem != nullptr)
    {
        OnlineSubsystem->ShowSteamInviteUI();
    }
}

