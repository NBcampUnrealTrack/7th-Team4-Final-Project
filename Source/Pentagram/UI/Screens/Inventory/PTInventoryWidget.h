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
<<<<<<< HEAD
    // 오버라이드
=======

>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
    virtual void NativeOnInitialized() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

<<<<<<< HEAD
    // 뒤로가기 여부
    UPROPERTY(BlueprintReadOnly, Category = "PT|Inventory")
    bool bIsBackHandler = false;
=======
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion")
};
