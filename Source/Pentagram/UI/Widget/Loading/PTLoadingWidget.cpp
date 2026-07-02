#include "UI/Widget/Loading/PTLoadingWidget.h"

#include "Core/Subsystems/PTLoadingSubsystem.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "UI/Setting/PTUISettings.h"

void UPTLoadingWidget::SetLoadingContext(FName InLoadingContext)
{
    LoadingContext = InLoadingContext;
    RefreshFromSubsystem();
}

float UPTLoadingWidget::GetLoadingProgress() const
{
    const UPTLoadingSubsystem* LoadingSubsystem = ResolveLoadingSubsystem();
    return LoadingSubsystem != nullptr ? LoadingSubsystem->GetLoadingProgress() : 0.f;
}

FText UPTLoadingWidget::GetLoadingStatusText() const
{
    return BuildStatusText(GetLoadingProgress());
}

void UPTLoadingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetVisibility(ESlateVisibility::HitTestInvisible);
    ApplyFullscreenCanvasSlot(Border_Dim, 0);
    ApplyFullscreenCanvasSlot(Image_Background, 1);
    HideLegacyWidgets();
    LoadBackgroundTextures();
    ApplyBackgroundTexture();
    SelectLoadingTip();
    RefreshFromSubsystem();
}

void UPTLoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    AdvanceBackgroundTexture(InDeltaTime);
    RefreshFromSubsystem();
}

UPTLoadingSubsystem* UPTLoadingWidget::ResolveLoadingSubsystem() const
{
    const UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr
        ? GameInstance->GetSubsystem<UPTLoadingSubsystem>()
        : nullptr;
}

void UPTLoadingWidget::LoadBackgroundTextures()
{
    BackgroundTexture = nullptr;
    BackgroundTextures.Reset();
    BackgroundTextureIndex = INDEX_NONE;
    BackgroundCycleElapsedTime = 0.f;

    const UPTUISettings* UISettings = GetDefault<UPTUISettings>();
    if (UISettings == nullptr)
    {
        return;
    }

    const TArray<TSoftObjectPtr<UTexture2D>>& BackgroundImages = UISettings->LoadingBackgroundImages;
    if (BackgroundImages.IsEmpty())
    {
        return;
    }

    BackgroundCycleInterval = FMath::Max(UISettings->LoadingBackgroundCycleInterval, 0.1f);

    const int32 StartImageIndex = UISettings->bRandomizeLoadingBackground
        ? FMath::RandRange(0, BackgroundImages.Num() - 1)
        : 0;

    for (int32 ImageOffset = 0; ImageOffset < BackgroundImages.Num(); ++ImageOffset)
    {
        const int32 ImageIndex = (StartImageIndex + ImageOffset) % BackgroundImages.Num();
        BackgroundTexture = BackgroundImages[ImageIndex].LoadSynchronous();
        if (BackgroundTexture != nullptr)
        {
            BackgroundTextures.Add(BackgroundTexture);
        }
    }

    if (BackgroundTextures.IsEmpty())
    {
        return;
    }

    BackgroundTextureIndex = 0;
    BackgroundTexture = BackgroundTextures[BackgroundTextureIndex];
}

void UPTLoadingWidget::ApplyBackgroundTexture()
{
    if (Image_Background == nullptr || BackgroundTexture == nullptr)
    {
        return;
    }

    Image_Background->SetBrushFromTexture(BackgroundTexture, true);
}

void UPTLoadingWidget::AdvanceBackgroundTexture(float DeltaTime)
{
    if (BackgroundTextures.Num() <= 1 || DeltaTime <= 0.f)
    {
        return;
    }

    BackgroundCycleElapsedTime += DeltaTime;
    if (BackgroundCycleElapsedTime < BackgroundCycleInterval)
    {
        return;
    }

    while (BackgroundCycleElapsedTime >= BackgroundCycleInterval)
    {
        BackgroundCycleElapsedTime -= BackgroundCycleInterval;
        BackgroundTextureIndex = (BackgroundTextureIndex + 1) % BackgroundTextures.Num();
    }

    BackgroundTexture = BackgroundTextures.IsValidIndex(BackgroundTextureIndex)
        ? BackgroundTextures[BackgroundTextureIndex]
        : nullptr;

    ApplyBackgroundTexture();
}

void UPTLoadingWidget::SelectLoadingTip()
{
    CurrentTipText = FText::GetEmpty();

    const UPTUISettings* UISettings = GetDefault<UPTUISettings>();
    if (UISettings == nullptr || UISettings->LoadingTips.IsEmpty())
    {
        return;
    }

    const int32 TipIndex = FMath::RandRange(0, UISettings->LoadingTips.Num() - 1);
    const FString& Tip = UISettings->LoadingTips[TipIndex];
    if (!Tip.IsEmpty())
    {
        CurrentTipText = FText::FromString(Tip);
    }
}

void UPTLoadingWidget::RefreshFromSubsystem()
{
    const UPTLoadingSubsystem* LoadingSubsystem = ResolveLoadingSubsystem();
    const float Progress = LoadingSubsystem != nullptr ? LoadingSubsystem->GetLoadingProgress() : 0.f;

    if (LoadingSubsystem != nullptr)
    {
        LoadingContext = LoadingSubsystem->GetLoadingContext();
    }

    RefreshBoundWidgets(Progress);
}

void UPTLoadingWidget::RefreshBoundWidgets(float Progress)
{
    const float ClampedProgress = FMath::Clamp(Progress, 0.f, 1.f);

    if (Image_ProgressFill != nullptr)
    {
        Image_ProgressFill->SetRenderTransformPivot(FVector2D(0.f, 0.5f));
        Image_ProgressFill->SetRenderScale(FVector2D(ClampedProgress, 1.f));
    }

    const FText StatusText = BuildStatusText(ClampedProgress);
    if (Text_LoadingStatus != nullptr)
    {
        Text_LoadingStatus->SetText(StatusText);
    }

    if (Text_Status != nullptr)
    {
        Text_Status->SetText(BuildTipText());
    }
}

void UPTLoadingWidget::ApplyFullscreenCanvasSlot(UWidget* Widget, int32 ZOrder) const
{
    if (Widget == nullptr)
    {
        return;
    }

    UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot);
    if (CanvasSlot == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Loading] %s is not a direct CanvasPanel child. Fullscreen slot fix skipped."),
            *GetNameSafe(Widget));
        return;
    }

    CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    CanvasSlot->SetAlignment(FVector2D::ZeroVector);
    CanvasSlot->SetAutoSize(false);
    CanvasSlot->SetOffsets(FMargin(0.f));
    CanvasSlot->SetZOrder(ZOrder);
}

void UPTLoadingWidget::HideLegacyWidgets()
{
    if (Border_Dim != nullptr)
    {
        Border_Dim->SetBrushColor(FLinearColor::Black);
        Border_Dim->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    if (Image_DividerLine != nullptr)
    {
        Image_DividerLine->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (Image_Compass != nullptr)
    {
        Image_Compass->SetVisibility(ESlateVisibility::Collapsed);
    }

}

FText UPTLoadingWidget::BuildStatusText(float Progress) const
{
    const int32 ProgressPercent = FMath::Clamp(FMath::RoundToInt(Progress * 100.f), 0, 100);

    if (LoadingContext == TEXT("TravelPreload"))
    {
        return FText::Format(
            NSLOCTEXT("PTLoading", "TravelPreload", "Preparing assets... {0}%"),
            FText::AsNumber(ProgressPercent));
    }

    if (LoadingContext == TEXT("MapTravel"))
    {
        return NSLOCTEXT("PTLoading", "MapTravel", "Loading map...");
    }

    return FText::Format(
        NSLOCTEXT("PTLoading", "DefaultLoading", "Loading... {0}%"),
        FText::AsNumber(ProgressPercent));
}

FText UPTLoadingWidget::BuildTipText() const
{
    if (!CurrentTipText.IsEmpty())
    {
        return CurrentTipText;
    }

    return NSLOCTEXT("PTLoading", "DefaultLoadingTip", "Tip: Better equipment helps you survive stronger enemies.");
}
