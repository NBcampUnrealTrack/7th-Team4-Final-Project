#include "PTIntroWidget.h"

#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Engine/LocalPlayer.h"
#include "UI/Manage/PTUIManagerSubsystem.h"

void UPTIntroWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetVisibility(ESlateVisibility::Visible);

    if (MediaPlayer)
    {
        MediaPlayer->OnEndReached.AddDynamic(this, &UPTIntroWidget::HandleEndReached);
    }

    Play();
}

void UPTIntroWidget::NativeDestruct()
{
    if (MediaPlayer)
    {
        MediaPlayer->OnEndReached.RemoveDynamic(this, &UPTIntroWidget::HandleEndReached);
        MediaPlayer->Close();
    }

    Super::NativeDestruct();
}

FReply UPTIntroWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    GoToMenu();
    return FReply::Handled();
}

TOptional<FUIInputConfig> UPTIntroWidget::GetDesiredInputConfig() const
{
    return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void UPTIntroWidget::Play()
{
    if (!MediaPlayer || !Video01)
    {
        return;
    }

    bLooping = false;
    MediaPlayer->SetLooping(false);
    MediaPlayer->OpenSource(Video01);
}

void UPTIntroWidget::HandleEndReached()
{
    // 1번 끝 -> 2번 루프
    if (bLooping || !Video02)
    {
        return;
    }

    bLooping = true;
    MediaPlayer->SetLooping(true);
    MediaPlayer->OpenSource(Video02);
}

void UPTIntroWidget::GoToMenu()
{
    if (bTransitioned) return;
    bTransitioned = true;

    if (MediaPlayer) MediaPlayer->Close();

    if (ULocalPlayer* LP = GetOwningLocalPlayer())
    {
        if (UPTUIManagerSubsystem* UIManager = LP->GetSubsystem<UPTUIManagerSubsystem>())
        {
            UIManager->OpenUILevel(MenuLevelName);
        }
    }
}
