#pragma once

#include "CoreMinimal.h"
#include "PTInventorySlotWidget.h"
#include "PTEquipSlotWidget.generated.h"

// 장비 전용 슬롯
UCLASS()
class PENTAGRAM_API UPTEquipSlotWidget : public UPTInventorySlotWidget
{
    GENERATED_BODY()

public:
    // 허용 타입
    UFUNCTION(BlueprintPure, Category = "PT|Equip")
    EItemType GetAllowedType() const { return AllowedType; }

    // 장착 가능?
    UFUNCTION(BlueprintPure, Category = "PT|Equip")
    bool CanAccept(const FItemData& Item) const
    {
        return Item.Item_Category == EItemCategory::Equipment
            && Item.Item_Type == AllowedType;
    }

protected:
    // 허용 타입
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Equip")
    EItemType AllowedType = EItemType::Weapon;
};
