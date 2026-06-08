#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Item/PTItemTypes.h"
#include "PTInventoryWidget.generated.h"

// class UUniformGridPanel;
// class UPTInventorySlotWidget;
// class UPTEquipSlotWidget;

UCLASS()
class PENTAGRAM_API UPTInventoryWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:

    virtual void NativeOnInitialized() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

};
