// Fill out your copyright notice in the Description page of Project Settings.

#include "PTEndingWidget.h"
#include "CommonTextBlock.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/Button.h"
#include "UI/Manage/PTUIManagerSubsystem.h"

void UPTEndingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Txt_EndingTitle)
    {
        Txt_EndingTitle->SetText(EndingTitle);
    }

    if (Txt_EndingBody)
    {
        Txt_EndingBody->SetText(EndingBody);
    }

    if (Btn_ReturnToIntro)
    {
        Btn_ReturnToIntro->OnClicked.AddDynamic(this, &UPTEndingWidget::HandleReturnToIntroClicked);
    }
}

void UPTEndingWidget::HandleReturnToIntroClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (APTPlayerController* PTPC = Cast<APTPlayerController>(PC))
        {
            PTPC->Server_ReturnToMainMenu();
        }
    }
}
