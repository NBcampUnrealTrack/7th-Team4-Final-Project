#include "PTInventoryWidget.h"

void UPTInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
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
    DeactivateWidget(); // ESC/B로 닫기
    return true;
}
