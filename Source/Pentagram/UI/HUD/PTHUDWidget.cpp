// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/PTHUDWidget.h"

#include "Character/Monsters/PTMonsterCharacter.h"
#include "Character/Player/PTPlayerController.h"
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
}


void UPTHUDWidget::NativeOnDeactivated()
{
    if (APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PC->OnMonsterTargeted.RemoveDynamic(this, &UPTHUDWidget::HandleMonsterTargeted);
    }

    Super::NativeOnDeactivated();
}

bool UPTHUDWidget::NativeOnHandleBackAction()
{
    return false;
}

