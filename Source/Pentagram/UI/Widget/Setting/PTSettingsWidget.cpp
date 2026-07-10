#include "UI/Widget/Setting/PTSettingsWidget.h"

#include "Character/Player/PTPlayerController.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/LocalPlayer.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Widget/Setting/PTAudioSettingsWidget.h"
#include "UI/Widget/Setting/PTControlSettingsWidget.h"
#include "UI/Widget/Setting/PTDisplaySettingsWidget.h"
#include "UI/Widget/Setting/PTQuitConfirmWidget.h"

namespace
{
template <typename WidgetType>
WidgetType* FindWidgetOfClass(UWidget* RootWidget)
{
    if (RootWidget == nullptr)
    {
        return nullptr;
    }

    if (WidgetType* TypedWidget = Cast<WidgetType>(RootWidget))
    {
        return TypedWidget;
    }

    UPanelWidget* PanelWidget = Cast<UPanelWidget>(RootWidget);
    if (PanelWidget == nullptr)
    {
        return nullptr;
    }

    const int32 ChildCount = PanelWidget->GetChildrenCount();
    for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
    {
        if (WidgetType* TypedChild = FindWidgetOfClass<WidgetType>(PanelWidget->GetChildAt(ChildIndex)))
        {
            return TypedChild;
        }
    }

    return nullptr;
}
}

UPTSettingsWidget::UPTSettingsWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsBackHandler = true;
}

void UPTSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BindButtons();

    if (const APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        if (PlayerController->IA_OpenSettings != nullptr)
        {
            FBindUIActionArgs BindArgs(
                PlayerController->IA_OpenSettings,
                false,
                FSimpleDelegate::CreateUObject(this, &UPTSettingsWidget::HandleToggleSettingsAction));
            RegisterUIActionBinding(BindArgs);
        }
    }

    SetActivePage(DefaultPage);
}

void UPTSettingsWidget::NativeDestruct()
{
    UnbindButtons();

    Super::NativeDestruct();
}

void UPTSettingsWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    SetActivePage(DefaultPage);
}

void UPTSettingsWidget::NativeOnDeactivated()
{
    if (APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PlayerController->SetGameplayInputBlockedByUI(false);
        PlayerController->RestoreGameplayInput();
    }

    Super::NativeOnDeactivated();
}

bool UPTSettingsWidget::NativeOnHandleBackAction()
{
    DeactivateWidget();
    return true;
}

void UPTSettingsWidget::HandleToggleSettingsAction()
{
    DeactivateWidget();
}

void UPTSettingsWidget::SetActivePage(EPTSettingsPage Page)
{
    ActivePage = Page;

    if (Switcher_SettingPages != nullptr)
    {
        if (UWidget* PageWidget = ResolvePageWidget(Page))
        {
            Switcher_SettingPages->SetActiveWidget(PageWidget);
        }
        else
        {
            Switcher_SettingPages->SetActiveWidgetIndex(ResolvePageIndex(Page));
        }
    }

    SetPageActiveState(EPTSettingsPage::Display, Page == EPTSettingsPage::Display);
    SetPageActiveState(EPTSettingsPage::Audio, Page == EPTSettingsPage::Audio);
    SetPageActiveState(EPTSettingsPage::Mouse, Page == EPTSettingsPage::Mouse);
    OnSettingsPageChanged(Page);
}

void UPTSettingsWidget::HandleDisplayClicked()
{
    SetActivePage(EPTSettingsPage::Display);
}

void UPTSettingsWidget::HandleAudioClicked()
{
    SetActivePage(EPTSettingsPage::Audio);
}

void UPTSettingsWidget::HandleMouseClicked()
{
    SetActivePage(EPTSettingsPage::Mouse);
}

void UPTSettingsWidget::HandleCloseClicked()
{
    DeactivateWidget();
}

void UPTSettingsWidget::HandleQuitGameClicked()
{
    if (QuitConfirmWidgetClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Settings] QuitConfirmWidgetClass is not configured."));
        return;
    }

    ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
    if (LocalPlayer == nullptr)
    {
        return;
    }

    UPTUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPTUIManagerSubsystem>();
    if (UIManager == nullptr)
    {
        return;
    }

    UIManager->PushWidget(QuitConfirmWidgetClass, EPTUILayer::Modal);
}

void UPTSettingsWidget::HandleResetDefaultsClicked()
{
    ResetActivePageSettingsToDefaults();
}

void UPTSettingsWidget::HandleApplySettingsClicked()
{
    ApplyActivePageSettings();
}

void UPTSettingsWidget::HandleConfirmSettingsClicked()
{
    ApplyActivePageSettings();
    DeactivateWidget();
}

void UPTSettingsWidget::BindButtons()
{
    if (Btn_Display != nullptr)
    {
        Btn_Display->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleDisplayClicked);
    }

    if (Btn_Audio != nullptr)
    {
        Btn_Audio->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleAudioClicked);
    }

    if (Btn_Mouse != nullptr)
    {
        Btn_Mouse->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleMouseClicked);
    }

    if (Btn_Close != nullptr)
    {
        Btn_Close->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleCloseClicked);
    }

    if (Btn_Back != nullptr)
    {
        Btn_Back->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleCloseClicked);
    }

    if (Btn_ResetDefaults != nullptr)
    {
        Btn_ResetDefaults->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleResetDefaultsClicked);
    }

    if (Btn_ApplySettings != nullptr)
    {
        Btn_ApplySettings->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleApplySettingsClicked);
    }

    if (Btn_ConfirmSettings != nullptr)
    {
        Btn_ConfirmSettings->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleConfirmSettingsClicked);
    }

    if (Btn_QuitGame != nullptr)
    {
        Btn_QuitGame->OnClicked.AddDynamic(this, &UPTSettingsWidget::HandleQuitGameClicked);
    }
}

void UPTSettingsWidget::UnbindButtons()
{
    if (Btn_Display != nullptr)
    {
        Btn_Display->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleDisplayClicked);
    }

    if (Btn_Audio != nullptr)
    {
        Btn_Audio->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleAudioClicked);
    }

    if (Btn_Mouse != nullptr)
    {
        Btn_Mouse->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleMouseClicked);
    }

    if (Btn_Close != nullptr)
    {
        Btn_Close->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleCloseClicked);
    }

    if (Btn_Back != nullptr)
    {
        Btn_Back->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleCloseClicked);
    }

    if (Btn_ResetDefaults != nullptr)
    {
        Btn_ResetDefaults->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleResetDefaultsClicked);
    }

    if (Btn_ApplySettings != nullptr)
    {
        Btn_ApplySettings->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleApplySettingsClicked);
    }

    if (Btn_ConfirmSettings != nullptr)
    {
        Btn_ConfirmSettings->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleConfirmSettingsClicked);
    }

    if (Btn_QuitGame != nullptr)
    {
        Btn_QuitGame->OnClicked.RemoveDynamic(this, &UPTSettingsWidget::HandleQuitGameClicked);
    }
}

void UPTSettingsWidget::ApplyActivePageSettings()
{
    UWidget* ActivePageWidget = ResolvePageWidget(ActivePage);

    switch (ActivePage)
    {
    case EPTSettingsPage::Display:
        if (UPTDisplaySettingsWidget* DisplaySettingsWidget = FindWidgetOfClass<UPTDisplaySettingsWidget>(ActivePageWidget))
        {
            DisplaySettingsWidget->ApplySettings();
        }
        break;
    case EPTSettingsPage::Audio:
        if (UPTAudioSettingsWidget* AudioSettingsWidget = FindWidgetOfClass<UPTAudioSettingsWidget>(ActivePageWidget))
        {
            AudioSettingsWidget->ApplySettings();
        }
        break;
    case EPTSettingsPage::Mouse:
        if (UPTControlSettingsWidget* ControlSettingsWidget = FindWidgetOfClass<UPTControlSettingsWidget>(ActivePageWidget))
        {
            ControlSettingsWidget->ApplySettings();
        }
        break;
    default:
        break;
    }
}

void UPTSettingsWidget::ResetActivePageSettingsToDefaults()
{
    UWidget* ActivePageWidget = ResolvePageWidget(ActivePage);

    switch (ActivePage)
    {
    case EPTSettingsPage::Display:
        if (UPTDisplaySettingsWidget* DisplaySettingsWidget = FindWidgetOfClass<UPTDisplaySettingsWidget>(ActivePageWidget))
        {
            DisplaySettingsWidget->ResetSettingsToDefaults();
        }
        break;
    case EPTSettingsPage::Audio:
        if (UPTAudioSettingsWidget* AudioSettingsWidget = FindWidgetOfClass<UPTAudioSettingsWidget>(ActivePageWidget))
        {
            AudioSettingsWidget->ResetSettingsToDefaults();
        }
        break;
    case EPTSettingsPage::Mouse:
        if (UPTControlSettingsWidget* ControlSettingsWidget = FindWidgetOfClass<UPTControlSettingsWidget>(ActivePageWidget))
        {
            ControlSettingsWidget->ResetSettingsToDefaults();
        }
        break;
    default:
        break;
    }
}

UWidget* UPTSettingsWidget::ResolvePageWidget(EPTSettingsPage Page) const
{
    if (Page == EPTSettingsPage::Display && Page_Display != nullptr)
    {
        return Page_Display;
    }

    if (Page == EPTSettingsPage::Audio && Page_Audio != nullptr)
    {
        return Page_Audio;
    }

    if (Page == EPTSettingsPage::Mouse && Page_Mouse != nullptr)
    {
        return Page_Mouse;
    }

    if (Switcher_SettingPages == nullptr)
    {
        return nullptr;
    }

    const int32 PageIndex = ResolvePageIndex(Page);
    return Switcher_SettingPages->GetChildrenCount() > PageIndex ? Switcher_SettingPages->GetChildAt(PageIndex) : nullptr;
}

int32 UPTSettingsWidget::ResolvePageIndex(EPTSettingsPage Page) const
{
    switch (Page)
    {
    case EPTSettingsPage::Audio:
        return 1;
    case EPTSettingsPage::Mouse:
        return 2;
    case EPTSettingsPage::Display:
    default:
        return 0;
    }
}

void UPTSettingsWidget::SetPageActiveState(EPTSettingsPage Page, bool bActive) const
{
    UWidget* PageWidget = ResolvePageWidget(Page);
    if (PageWidget == nullptr)
    {
        return;
    }

    PageWidget->SetVisibility(bActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

    if (UCommonActivatableWidget* ActivatablePage = Cast<UCommonActivatableWidget>(PageWidget))
    {
        if (bActive)
        {
            ActivatablePage->ActivateWidget();
        }
        else
        {
            ActivatablePage->DeactivateWidget();
        }
    }
}
