#include "PTLobbyWidget.h"
#include "PTLobbySlotWidget.h"
#include "PTLobbyPreviewActor.h"
#include "Core/PTGameState.h"
#include "Core/Subsystems/PTOnlineSubsystem.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/GameInstance.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/Manage/PTUIManagerSubsystem.h"

void UPTLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ReadyButton && !ReadyButton->OnClicked.IsAlreadyBound(this, &UPTLobbyWidget::OnReadyClicked))
    {
        ReadyButton->OnClicked.AddDynamic(this, &UPTLobbyWidget::OnReadyClicked);
    }

    if (InviteButton && !InviteButton->OnClicked.IsAlreadyBound(this, &UPTLobbyWidget::OnInviteClicked))
    {
        InviteButton->OnClicked.AddDynamic(this, &UPTLobbyWidget::OnInviteClicked);
        
    if (LeaveButton && !LeaveButton->OnClicked.IsAlreadyBound(this, &UPTLobbyWidget::OnLeaveClicked))
    {
        LeaveButton->OnClicked.AddDynamic(this, &UPTLobbyWidget::OnLeaveClicked);
    }

    BindGameState();
    SpawnPreview();
    RefreshPreview();
}

void UPTLobbyWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BindRetryTimer);

        if (APTGameState* PTGameState = World->GetGameState<APTGameState>())
        {
            PTGameState->OnLobbyUpdated.RemoveDynamic(this, &UPTLobbyWidget::HandleLobbyUpdated);
        }
    }

    if (ReadyButton)
    {
        ReadyButton->OnClicked.RemoveDynamic(this, &UPTLobbyWidget::OnReadyClicked);
    }

    if (InviteButton)
    {
        InviteButton->OnClicked.RemoveDynamic(this, &UPTLobbyWidget::OnInviteClicked);
    }

    if (LeaveButton)
    {
        LeaveButton->OnClicked.RemoveDynamic(this, &UPTLobbyWidget::OnLeaveClicked);
    }
    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }

    Super::NativeDestruct();
}

void UPTLobbyWidget::BindGameState()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    APTGameState* PTGameState = World->GetGameState<APTGameState>();
    if (PTGameState == nullptr)
    {
        // GS 대기 재시도
        World->GetTimerManager().SetTimer(
            BindRetryTimer, this, &UPTLobbyWidget::BindGameState, 0.2f, false);
        return;
    }

    if (!PTGameState->OnLobbyUpdated.IsAlreadyBound(this, &UPTLobbyWidget::HandleLobbyUpdated))
    {
        PTGameState->OnLobbyUpdated.AddDynamic(this, &UPTLobbyWidget::HandleLobbyUpdated);
    }

    RefreshLobby();
}

void UPTLobbyWidget::HandleLobbyUpdated()
{
    RefreshLobby();
}

void UPTLobbyWidget::RefreshLobby()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    APTGameState* PTGameState = World->GetGameState<APTGameState>();
    if (PTGameState == nullptr)
    {
        return;
    }

    UPTLobbySlotWidget* Slots[4] =
    {
        PlayerSlot_0, PlayerSlot_1, PlayerSlot_2, PlayerSlot_3
    };

    const int32 PlayerCount = PTGameState->PlayerArray.Num();

    for (int32 Index = 0; Index < 4; ++Index)
    {
        if (Slots[Index] == nullptr)
        {
            continue;
        }

        if (Index < PlayerCount)
        {
            if (APTBasePlayerState* PS = Cast<APTBasePlayerState>(PTGameState->PlayerArray[Index]))
            {
                Slots[Index]->SetSlot(PS->GetPlayerName(), PS->PlayerLevel, PS->IsReady());
                continue;
            }
        }

        Slots[Index]->SetEmpty();
    }
}

void UPTLobbyWidget::SpawnPreview()
{
    if (PreviewActor != nullptr || PreviewActorClass == nullptr)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    FActorSpawnParameters SpawnParameters;
    PreviewActor = World->SpawnActor<APTLobbyPreviewActor>(
        PreviewActorClass, PreviewSpawnTransform, SpawnParameters);
}

void UPTLobbyWidget::RefreshPreview()
{
    if (CharacterPreviewImage == nullptr || PreviewActor == nullptr)
    {
        return;
    }

    UTextureRenderTarget2D* RenderTarget = PreviewActor->GetRenderTarget();
    if (RenderTarget == nullptr || PreviewMaterial == nullptr)
    {
        return;
    }

    UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(PreviewMaterial, this);
    MID->SetTextureParameterValue(TEXT("PreviewTex"), RenderTarget);  // 머티리얼에 RT 주입
    CharacterPreviewImage->SetBrushFromMaterial(MID);
}

void UPTLobbyWidget::OnReadyClicked()
{
    bLocalReady = !bLocalReady;

    if (APTPlayerController* PTController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PTController->Server_SetReady(bLocalReady);
    }

    if (ReadyButtonText)
    {
        ReadyButtonText->SetText(FText::FromString(bLocalReady ? TEXT("준비 취소") : TEXT("준비")));
    }
}

void UPTLobbyWidget::OnInviteClicked()
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

void UPTLobbyWidget::OnLeaveClicked()
{
    // 방법 1) 네트워크 세션에서 완전히 나가기 (서버 접속 해제 후 로컬 MainMenu로)
    // 나중에 로비가 실제 서버 세션이라 접속 해제가 필요해지면 아래로 교체
    //
    // if (APlayerController* PC = GetOwningPlayer())
    // {
    //     PC->ClientTravel(TEXT("/Game/Maps/L_Intro"), ETravelType::TRAVEL_Absolute);
    // }

    if (APTPlayerController* PTController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        // 나가면서 준비 상태 초기화
        bLocalReady = false;
        PTController->Server_SetReady(false);
    }

    ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
    if (LocalPlayer == nullptr)
    {
        return;
    }

    if (UPTUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPTUIManagerSubsystem>())
    {
        UIManager->OpenUILevel(FName("L_MainMenu"));
    }
}
