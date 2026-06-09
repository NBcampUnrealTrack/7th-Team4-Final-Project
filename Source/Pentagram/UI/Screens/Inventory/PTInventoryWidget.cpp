#include "PTInventoryWidget.h"
#include "Input/CommonUIInputTypes.h"

void UPTInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

TOptional<FUIInputConfig> UPTInventoryWidget::GetDesiredInputConfig() const
{
    return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture);
}

void UPTInventoryWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
}

void UPTInventoryWidget::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
}

bool UPTInventoryWidget::NativeOnHandleBackAction()
{
    bIsBackHandler = true;
    DeactivateWidget(); // 닫기
    return true;
}
