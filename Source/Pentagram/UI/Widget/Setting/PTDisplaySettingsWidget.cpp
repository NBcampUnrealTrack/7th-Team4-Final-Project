#include "UI/Widget/Setting/PTDisplaySettingsWidget.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
constexpr float DefaultBrightness = 0.5f;
constexpr float MinBrightnessGamma = 1.6f;
constexpr float MaxBrightnessGamma = 2.8f;

const TCHAR* DisplaySettingsSection = TEXT("/Script/Pentagram.PTDisplaySettings");
const TCHAR* BrightnessKey = TEXT("Brightness");
const TCHAR* UnlimitedFrameLimitLabel = TEXT("제한 없음");
const TCHAR* CustomQualityLabel = TEXT("사용자 지정");

int32 CalculateGreatestCommonDivisor(int32 Left, int32 Right)
{
    Left = FMath::Abs(Left);
    Right = FMath::Abs(Right);

    while (Right != 0)
    {
        const int32 Remainder = Left % Right;
        Left = Right;
        Right = Remainder;
    }

    return FMath::Max(Left, 1);
}

FString FormatNumberWithComma(int32 Value)
{
    FString RawText = FString::FromInt(Value);
    FString FormattedText;

    int32 DigitCount = 0;
    for (int32 Index = RawText.Len() - 1; Index >= 0; --Index)
    {
        if (DigitCount > 0 && DigitCount % 3 == 0)
        {
            FormattedText.InsertAt(0, TEXT(","));
        }

        FormattedText.InsertAt(0, RawText[Index]);
        ++DigitCount;
    }

    return FormattedText;
}

FString MakeAspectRatioLabel(const FIntPoint& Resolution)
{
    if (Resolution.X <= 0 || Resolution.Y <= 0)
    {
        return TEXT("");
    }

    const int32 Divisor = CalculateGreatestCommonDivisor(Resolution.X, Resolution.Y);
    return FString::Printf(TEXT("%d:%d"), Resolution.X / Divisor, Resolution.Y / Divisor);
}

float BrightnessToGamma(float Brightness)
{
    const float ClampedBrightness = FMath::Clamp(Brightness, 0.f, 1.f);
    return FMath::Lerp(MaxBrightnessGamma, MinBrightnessGamma, ClampedBrightness);
}
}

void UPTDisplaySettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    PopulateOptionLists();

    if (Combo_WindowMode != nullptr)
    {
        Combo_WindowMode->OnSelectionChanged.AddDynamic(this, &UPTDisplaySettingsWidget::HandleWindowModeChanged);
    }

    if (Combo_Resolution != nullptr)
    {
        Combo_Resolution->OnSelectionChanged.AddDynamic(this, &UPTDisplaySettingsWidget::HandleResolutionChanged);
    }

    if (CheckBox_VSync != nullptr)
    {
        CheckBox_VSync->OnCheckStateChanged.AddDynamic(this, &UPTDisplaySettingsWidget::HandleVSyncChanged);
    }

    if (Combo_FrameLimit != nullptr)
    {
        Combo_FrameLimit->OnSelectionChanged.AddDynamic(this, &UPTDisplaySettingsWidget::HandleFrameLimitChanged);
    }

    if (Combo_Quality != nullptr)
    {
        Combo_Quality->OnSelectionChanged.AddDynamic(this, &UPTDisplaySettingsWidget::HandleQualityChanged);
    }

    if (Slider_Brightness != nullptr)
    {
        Slider_Brightness->OnValueChanged.AddDynamic(this, &UPTDisplaySettingsWidget::HandleBrightnessChanged);
    }

    RefreshFromGameUserSettings();
}

void UPTDisplaySettingsWidget::NativeDestruct()
{
    if (Combo_WindowMode != nullptr)
    {
        Combo_WindowMode->OnSelectionChanged.RemoveDynamic(this, &UPTDisplaySettingsWidget::HandleWindowModeChanged);
    }

    if (Combo_Resolution != nullptr)
    {
        Combo_Resolution->OnSelectionChanged.RemoveDynamic(this, &UPTDisplaySettingsWidget::HandleResolutionChanged);
    }

    if (CheckBox_VSync != nullptr)
    {
        CheckBox_VSync->OnCheckStateChanged.RemoveDynamic(this, &UPTDisplaySettingsWidget::HandleVSyncChanged);
    }

    if (Combo_FrameLimit != nullptr)
    {
        Combo_FrameLimit->OnSelectionChanged.RemoveDynamic(this, &UPTDisplaySettingsWidget::HandleFrameLimitChanged);
    }

    if (Combo_Quality != nullptr)
    {
        Combo_Quality->OnSelectionChanged.RemoveDynamic(this, &UPTDisplaySettingsWidget::HandleQualityChanged);
    }

    if (Slider_Brightness != nullptr)
    {
        Slider_Brightness->OnValueChanged.RemoveDynamic(this, &UPTDisplaySettingsWidget::HandleBrightnessChanged);
    }

    Super::NativeDestruct();
}

void UPTDisplaySettingsWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    RefreshFromGameUserSettings();
}

void UPTDisplaySettingsWidget::HandleWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bIsRefreshing)
    {
        return;
    }

    if (const EWindowMode::Type* WindowMode = WindowModeOptions.Find(SelectedItem))
    {
        PendingWindowMode = *WindowMode;
        ApplyDisplaySettings();
    }
}

void UPTDisplaySettingsWidget::HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bIsRefreshing)
    {
        return;
    }

    if (const FIntPoint* Resolution = ResolutionOptions.Find(SelectedItem))
    {
        PendingResolution = *Resolution;
        ApplyDisplaySettings();
    }
}

void UPTDisplaySettingsWidget::HandleVSyncChanged(bool bIsChecked)
{
    if (bIsRefreshing)
    {
        return;
    }

    bPendingVSync = bIsChecked;
    ApplyDisplaySettings();
}

void UPTDisplaySettingsWidget::HandleFrameLimitChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bIsRefreshing)
    {
        return;
    }

    if (const float* FrameLimit = FrameLimitOptions.Find(SelectedItem))
    {
        PendingFrameLimit = *FrameLimit;
        ApplyDisplaySettings();
    }
}

void UPTDisplaySettingsWidget::HandleQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bIsRefreshing)
    {
        return;
    }

    if (const int32* QualityLevel = QualityOptions.Find(SelectedItem))
    {
        PendingQualityLevel = *QualityLevel;
        ApplyDisplaySettings();
    }
}

void UPTDisplaySettingsWidget::HandleBrightnessChanged(float Value)
{
    if (bIsRefreshing)
    {
        return;
    }

    PendingBrightness = QuantizeNormalizedValue(Value);

    bIsRefreshing = true;
    if (Slider_Brightness != nullptr)
    {
        Slider_Brightness->SetValue(PendingBrightness);
    }
    bIsRefreshing = false;

    SetBrightnessDisplay(PendingBrightness);
}

void UPTDisplaySettingsWidget::ApplySettings()
{
    ApplyDisplaySettings();
}

void UPTDisplaySettingsWidget::ResetSettingsToDefaults()
{
    if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
    {
        GameUserSettings->SetToDefaults();
        GameUserSettings->ApplySettings(false);
        GameUserSettings->SaveSettings();
    }

    SaveBrightness(DefaultBrightness);
    ApplyBrightness(DefaultBrightness);
    PopulateOptionLists();
    RefreshFromGameUserSettings();
}

void UPTDisplaySettingsWidget::PopulateOptionLists()
{
    PopulateWindowModeOptions();
    PopulateResolutionOptions();
    PopulateFrameLimitOptions();
    PopulateQualityOptions();
}

void UPTDisplaySettingsWidget::PopulateWindowModeOptions()
{
    WindowModeOptions.Reset();

    if (Combo_WindowMode != nullptr)
    {
        Combo_WindowMode->ClearOptions();
    }

    const TArray<TPair<FString, EWindowMode::Type>> Options =
    {
        TPair<FString, EWindowMode::Type>(TEXT("전체 화면"), EWindowMode::Fullscreen),
        TPair<FString, EWindowMode::Type>(TEXT("테두리 없는 창"), EWindowMode::WindowedFullscreen),
        TPair<FString, EWindowMode::Type>(TEXT("창 모드"), EWindowMode::Windowed)
    };

    for (const TPair<FString, EWindowMode::Type>& Option : Options)
    {
        WindowModeOptions.Add(Option.Key, Option.Value);

        if (Combo_WindowMode != nullptr)
        {
            Combo_WindowMode->AddOption(Option.Key);
        }
    }
}

void UPTDisplaySettingsWidget::PopulateResolutionOptions()
{
    ResolutionOptions.Reset();

    if (Combo_Resolution != nullptr)
    {
        Combo_Resolution->ClearOptions();
    }

    TArray<FIntPoint> Resolutions;
    UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

    if (Resolutions.IsEmpty())
    {
        Resolutions.Add(FIntPoint(1280, 720));
        Resolutions.Add(FIntPoint(1920, 1080));
        Resolutions.Add(FIntPoint(2560, 1440));
    }

    Resolutions.Sort([](const FIntPoint& Left, const FIntPoint& Right)
    {
        return Left.X == Right.X ? Left.Y < Right.Y : Left.X < Right.X;
    });

    TSet<FString> AddedLabels;
    for (const FIntPoint& Resolution : Resolutions)
    {
        if (Resolution.X <= 0 || Resolution.Y <= 0)
        {
            continue;
        }

        const FString Label = MakeResolutionLabel(Resolution);
        if (AddedLabels.Contains(Label))
        {
            continue;
        }

        AddedLabels.Add(Label);
        ResolutionOptions.Add(Label, Resolution);

        if (Combo_Resolution != nullptr)
        {
            Combo_Resolution->AddOption(Label);
        }
    }
}

void UPTDisplaySettingsWidget::PopulateFrameLimitOptions()
{
    FrameLimitOptions.Reset();

    if (Combo_FrameLimit != nullptr)
    {
        Combo_FrameLimit->ClearOptions();
    }

    const TArray<float> FrameLimits =
    {
        0.f,
        30.f,
        60.f,
        120.f,
        144.f,
        165.f,
        240.f
    };

    for (float FrameLimit : FrameLimits)
    {
        const FString Label = MakeFrameLimitLabel(FrameLimit);
        FrameLimitOptions.Add(Label, FrameLimit);

        if (Combo_FrameLimit != nullptr)
        {
            Combo_FrameLimit->AddOption(Label);
        }
    }
}

void UPTDisplaySettingsWidget::PopulateQualityOptions()
{
    QualityOptions.Reset();

    if (Combo_Quality != nullptr)
    {
        Combo_Quality->ClearOptions();
    }

    for (int32 QualityLevel = 0; QualityLevel <= 4; ++QualityLevel)
    {
        const FString Label = MakeQualityLabel(QualityLevel);
        QualityOptions.Add(Label, QualityLevel);

        if (Combo_Quality != nullptr)
        {
            Combo_Quality->AddOption(Label);
        }
    }
}

void UPTDisplaySettingsWidget::RefreshFromGameUserSettings()
{
    UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
    if (GameUserSettings == nullptr)
    {
        return;
    }

    GameUserSettings->LoadSettings(false);

    PendingWindowMode = GameUserSettings->GetFullscreenMode();
    PendingResolution = GameUserSettings->GetScreenResolution();
    bPendingVSync = GameUserSettings->IsVSyncEnabled();
    PendingFrameLimit = GameUserSettings->GetFrameRateLimit();
    PendingQualityLevel = GameUserSettings->GetOverallScalabilityLevel();
    PendingBrightness = QuantizeNormalizedValue(LoadBrightness());

    const FString ResolutionLabel = MakeResolutionLabel(PendingResolution);
    if (!ResolutionOptions.Contains(ResolutionLabel))
    {
        ResolutionOptions.Add(ResolutionLabel, PendingResolution);

        if (Combo_Resolution != nullptr)
        {
            Combo_Resolution->AddOption(ResolutionLabel);
        }
    }

    const FString FrameLimitLabel = MakeFrameLimitLabel(PendingFrameLimit);
    if (!FrameLimitOptions.Contains(FrameLimitLabel))
    {
        FrameLimitOptions.Add(FrameLimitLabel, PendingFrameLimit);

        if (Combo_FrameLimit != nullptr)
        {
            Combo_FrameLimit->AddOption(FrameLimitLabel);
        }
    }

    const FString QualityLabel = MakeQualityLabel(PendingQualityLevel);
    if (!QualityOptions.Contains(QualityLabel))
    {
        QualityOptions.Add(QualityLabel, PendingQualityLevel);

        if (Combo_Quality != nullptr)
        {
            Combo_Quality->AddOption(QualityLabel);
        }
    }

    bIsRefreshing = true;
    SetSelectedOption(Combo_WindowMode, MakeWindowModeLabel(PendingWindowMode));
    SetSelectedOption(Combo_Resolution, ResolutionLabel);
    SetSelectedOption(Combo_FrameLimit, FrameLimitLabel);
    SetSelectedOption(Combo_Quality, QualityLabel);

    if (CheckBox_VSync != nullptr)
    {
        CheckBox_VSync->SetIsChecked(bPendingVSync);
    }

    if (Slider_Brightness != nullptr)
    {
        Slider_Brightness->SetValue(PendingBrightness);
    }
    bIsRefreshing = false;

    ApplyBrightness(PendingBrightness);
    SetBrightnessDisplay(PendingBrightness);
}

void UPTDisplaySettingsWidget::ApplyDisplaySettings()
{
    UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
    if (GameUserSettings == nullptr)
    {
        return;
    }

    GameUserSettings->SetFullscreenMode(PendingWindowMode);
    GameUserSettings->SetScreenResolution(PendingResolution);
    GameUserSettings->SetVSyncEnabled(bPendingVSync);
    GameUserSettings->SetFrameRateLimit(PendingFrameLimit);

    if (PendingQualityLevel >= 0)
    {
        GameUserSettings->SetOverallScalabilityLevel(PendingQualityLevel);
    }

    GameUserSettings->ApplySettings(false);
    GameUserSettings->SaveSettings();

    ApplyBrightness(PendingBrightness);
    SaveBrightness(PendingBrightness);
}

void UPTDisplaySettingsWidget::ApplyBrightness(float Brightness) const
{
    IConsoleVariable* TonemapperGamma = IConsoleManager::Get().FindConsoleVariable(TEXT("r.TonemapperGamma"));
    if (TonemapperGamma != nullptr)
    {
        TonemapperGamma->Set(BrightnessToGamma(Brightness), ECVF_SetByGameSetting);
    }
}

float UPTDisplaySettingsWidget::LoadBrightness() const
{
    float Brightness = DefaultBrightness;

    if (GConfig != nullptr)
    {
        GConfig->GetFloat(DisplaySettingsSection, BrightnessKey, Brightness, GGameUserSettingsIni);
    }

    return FMath::Clamp(Brightness, 0.f, 1.f);
}

void UPTDisplaySettingsWidget::SaveBrightness(float Brightness) const
{
    if (GConfig == nullptr)
    {
        return;
    }

    GConfig->SetFloat(DisplaySettingsSection, BrightnessKey, FMath::Clamp(Brightness, 0.f, 1.f), GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

FString UPTDisplaySettingsWidget::MakeResolutionLabel(const FIntPoint& Resolution) const
{
    return FString::Printf(
        TEXT("%s x %s %s"),
        *FormatNumberWithComma(Resolution.X),
        *FormatNumberWithComma(Resolution.Y),
        *MakeAspectRatioLabel(Resolution));
}

FString UPTDisplaySettingsWidget::MakeFrameLimitLabel(float FrameLimit) const
{
    if (FrameLimit <= 0.1f)
    {
        return UnlimitedFrameLimitLabel;
    }

    return FString::FromInt(FMath::RoundToInt(FrameLimit));
}

FString UPTDisplaySettingsWidget::MakeQualityLabel(int32 QualityLevel) const
{
    switch (QualityLevel)
    {
    case 0:
        return TEXT("낮음");
    case 1:
        return TEXT("중간");
    case 2:
        return TEXT("높음");
    case 3:
        return TEXT("에픽");
    case 4:
        return TEXT("시네마틱");
    default:
        return CustomQualityLabel;
    }
}

FString UPTDisplaySettingsWidget::MakeWindowModeLabel(EWindowMode::Type WindowMode) const
{
    switch (WindowMode)
    {
    case EWindowMode::Fullscreen:
        return TEXT("전체 화면");
    case EWindowMode::Windowed:
        return TEXT("창 모드");
    case EWindowMode::WindowedFullscreen:
    default:
        return TEXT("테두리 없는 창");
    }
}

float UPTDisplaySettingsWidget::QuantizeNormalizedValue(float Value) const
{
    return FMath::Clamp(FMath::RoundToFloat(Value * 100.f) / 100.f, 0.f, 1.f);
}

void UPTDisplaySettingsWidget::SetSelectedOption(UComboBoxString* ComboBox, const FString& Option) const
{
    if (ComboBox != nullptr && ComboBox->FindOptionIndex(Option) != INDEX_NONE)
    {
        ComboBox->SetSelectedOption(Option);
    }
}

void UPTDisplaySettingsWidget::SetBrightnessDisplay(float Brightness) const
{
    const float ClampedBrightness = FMath::Clamp(Brightness, 0.f, 1.f);

    if (Progress_Brightness != nullptr)
    {
        Progress_Brightness->SetPercent(ClampedBrightness);
    }

    if (Text_BrightnessValue != nullptr)
    {
        Text_BrightnessValue->SetText(FText::AsNumber(FMath::RoundToInt(ClampedBrightness * 100.f)));
    }
}
