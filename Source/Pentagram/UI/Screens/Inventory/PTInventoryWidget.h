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
    // 오버라이드
    virtual void NativeOnInitialized() override;
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;
    //~ End UCommonActivatableWidget

    // 뒤로가기 여부
    UPROPERTY(BlueprintReadOnly, Category = "PT|Inventory")
    bool bIsBackHandler = false;
};
