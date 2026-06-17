#include "PTLobbyWidget.h"

#include "PTLobbySlotWidget.h"
#include "PTLobbyPreviewActor.h"
#include "Core/PTGameState.h"                       // 경로는 프로젝트에 맞게
#include "Character/Player/PTBasePlayerState.h"     // 경로는 프로젝트에 맞게
#include "Character/Player/PTPlayerController.h"     // 경로는 프로젝트에 맞게
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UPTLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ReadyButton && !ReadyButton->OnClicked.IsAlreadyBound(this, &UPTLobbyWidget::OnReadyClicked))
    {
        ReadyButton->OnClicked.AddDynamic(this, &UPTLobbyWidget::OnReadyClicked);
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

    UPTLobbySlotWidget* Slots[5] =
    {
        PlayerSlot_0, PlayerSlot_1, PlayerSlot_2, PlayerSlot_3, PlayerSlot_4
    };

    const int32 PlayerCount = PTGameState->PlayerArray.Num();

    for (int32 Index = 0; Index < 5; ++Index)
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

    if (UTextureRenderTarget2D* RenderTarget = PreviewActor->GetRenderTarget())
    {
        CharacterPreviewImage->SetBrushResourceObject(RenderTarget);
    }
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
