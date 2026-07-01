#include "UI/Widget/Loading/PTLoadingWidget.h"

#include "Core/Subsystems/PTLoadingSubsystem.h"
#include "Engine/GameInstance.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"

void UPTLoadingWidget::SetLoadingContext(FName InLoadingContext)
{
    LoadingContext = InLoadingContext;
    RefreshFromSubsystem();
}

TSharedRef<SWidget> UPTLoadingWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SBorder)
            .BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.88f))
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SBox)
            .WidthOverride(520.f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                [
                    SAssignNew(SlateStatusText, STextBlock)
                    .Text(FText::FromString(TEXT("Loading...")))
                    .ColorAndOpacity(FSlateColor(FLinearColor::White))
                    .Justification(ETextJustify::Center)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.f, 24.f, 0.f, 0.f)
                [
                    SAssignNew(SlateProgressBar, SProgressBar)
                    .Percent(TOptional<float>(0.f))
                ]
            ]
        ];
}

void UPTLoadingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetVisibility(ESlateVisibility::HitTestInvisible);
    RefreshFromSubsystem();
}

void UPTLoadingWidget::NativeDestruct()
{
    SlateProgressBar.Reset();
    SlateStatusText.Reset();

    Super::NativeDestruct();
}

void UPTLoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    RefreshFromSubsystem();
}

UPTLoadingSubsystem* UPTLoadingWidget::ResolveLoadingSubsystem() const
{
    const UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr
        ? GameInstance->GetSubsystem<UPTLoadingSubsystem>()
        : nullptr;
}

void UPTLoadingWidget::RefreshFromSubsystem()
{
    const UPTLoadingSubsystem* LoadingSubsystem = ResolveLoadingSubsystem();
    const float Progress = LoadingSubsystem != nullptr ? LoadingSubsystem->GetLoadingProgress() : 0.f;

    if (LoadingSubsystem != nullptr)
    {
        LoadingContext = LoadingSubsystem->GetLoadingContext();
    }

    if (SlateProgressBar.IsValid())
    {
        SlateProgressBar->SetPercent(TOptional<float>(Progress));
    }

    if (SlateStatusText.IsValid())
    {
        SlateStatusText->SetText(BuildStatusText(Progress));
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
