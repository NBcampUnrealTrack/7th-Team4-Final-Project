#include "PTNPCInteractionPromptWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/NPC/PTNPCCharacter.h"
#include "GameFramework/PlayerController.h"

UPTNPCInteractionPromptWidget::UPTNPCInteractionPromptWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(false);
    SetVisibility(ESlateVisibility::Collapsed);
}

void UPTNPCInteractionPromptWidget::SetupPrompt(APTNPCCharacter* InNPC)
{
    TargetNPC = InNPC;
    UpdatePromptPosition();
}

void UPTNPCInteractionPromptWidget::ShowPrompt()
{
    SetVisibility(ESlateVisibility::HitTestInvisible);
    UpdatePromptPosition();
}

void UPTNPCInteractionPromptWidget::HidePrompt()
{
    SetVisibility(ESlateVisibility::Collapsed);
    TargetNPC = nullptr;
}

void UPTNPCInteractionPromptWidget::NativeDestruct()
{
    TargetNPC = nullptr;

    Super::NativeDestruct();
}

void UPTNPCInteractionPromptWidget::UpdatePromptPosition()
{
    if (TargetNPC == nullptr)
    {
        return;
    }

    APlayerController* PlayerController = GetOwningPlayer();
    if (PlayerController == nullptr)
    {
        return;
    }

    FVector2D WidgetPosition;
    const FVector WorldLocation = TargetNPC->GetActorLocation() + PromptWorldOffset;
    const bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
        PlayerController,
        WorldLocation,
        WidgetPosition,
        true);

    if (!bProjected)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    SetAlignmentInViewport(FVector2D(0.5f, 1.f));
    SetPositionInViewport(WidgetPosition + PromptScreenOffset, false);
}
