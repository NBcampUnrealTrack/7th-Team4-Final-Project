#include "PTMainMenuWidget.h"

#include "Components/Button.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Manage/PTUIManagerSubsystem.h"

void UPTMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    PlayMenuVideo();

    if (Btn_GameStart)
    {
        Btn_GameStart->OnClicked.AddDynamic(this, &UPTMainMenuWidget::HandleGameStartClicked);
    }

    if (Btn_Quit)
    {
        Btn_Quit->OnClicked.AddDynamic(this, &UPTMainMenuWidget::HandleQuitClicked);
    }
}

void UPTMainMenuWidget::NativeDestruct()
{
    if (Btn_GameStart)
    {
        Btn_GameStart->OnClicked.RemoveDynamic(this, &UPTMainMenuWidget::HandleGameStartClicked);
    }

    if (Btn_Quit)
    {
        Btn_Quit->OnClicked.RemoveDynamic(this, &UPTMainMenuWidget::HandleQuitClicked);
    }

    if (MediaPlayer)
    {
        MediaPlayer->Close();
    }

    Super::NativeDestruct();
}

void UPTMainMenuWidget::PlayMenuVideo()
{
    if (!MediaPlayer || !MenuVideo) return;

    MediaPlayer->SetLooping(true);
    MediaPlayer->OpenSource(MenuVideo);
}

void UPTMainMenuWidget::HandleGameStartClicked()
{
    // 서브레벨 이동 (보류)
    // ULocalPlayer* LP = GetOwningLocalPlayer();
    // if (!LP) return;
    //
    // UPTUIManagerSubsystem* UIManager = LP->GetSubsystem<UPTUIManagerSubsystem>();
    // if (!UIManager) return;
    //
    // UIManager->OpenUILevel(LobbyLevelName);

    // 임시 트레블
    if (UWorld* World = GetWorld())
    {
        // TODO: 테스트 후 삭제, 경로만 교체
        World->ServerTravel(TEXT("/Game/Pentagram/Level/L_Lobby"));
    }
}

void UPTMainMenuWidget::HandleQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
