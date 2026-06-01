// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTInventorySlotWidget.h"
#include "PTEquipSlotWidget.generated.h"

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTEquipSlotWidget : public UPTInventorySlotWidget
{
    GENERATED_BODY()

public:
    /** 이 슬롯에 들어올 수 있는 타입 (Weapon / Chest). BP에서 슬롯별로 지정. */
    UFUNCTION(BlueprintPure, Category = "PT|Equip")
    EItemType GetAllowedType() const { return AllowedType; }

    /** 해당 아이템이 이 장비 슬롯에 장착 가능한지. */
    UFUNCTION(BlueprintPure, Category = "PT|Equip")
    bool CanAccept(const FItemData& Item) const
    {
        return Item.Item_Category == EItemCategory::Equipment
            && Item.Item_Type == AllowedType;
    }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Equip")
    EItemType AllowedType = EItemType::Weapon;
};
