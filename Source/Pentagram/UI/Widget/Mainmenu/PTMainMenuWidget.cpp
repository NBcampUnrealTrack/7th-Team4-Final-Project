#include "PTMainMenuWidget.h"

#include "Components/Button.h"
#include "Core/Subsystems/PTOnlineSubsystem.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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
    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
    UPTOnlineSubsystem* OnlineSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTOnlineSubsystem>() : nullptr;
    if (OnlineSubsystem != nullptr)
    {
        OnlineSubsystem->HostSteamSession(LobbyLevelName, MaxLobbyPlayers, bShowSteamInviteUIAfterHost);
        return;
    }

    if (World != nullptr)
    {
        UGameplayStatics::OpenLevel(World, LobbyLevelName, true, TEXT("listen"));
    }
}

void UPTMainMenuWidget::HandleQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
