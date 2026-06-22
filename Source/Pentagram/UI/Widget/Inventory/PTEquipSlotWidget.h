#pragma once

#include "CoreMinimal.h"
#include "PTInventorySlotWidget.h"
#include "PTEquipSlotWidget.generated.h"

class UImage;
class UDragDropOperation;

// 장착 요청
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipRequested, int32, FromIndex, EItemType, EquipType);

// 장비 전용 슬롯
UCLASS()
class PENTAGRAM_API UPTEquipSlotWidget : public UPTInventorySlotWidget
{
    GENERATED_BODY()

public:
    // ── 델리게이트 ──
    UPROPERTY(BlueprintAssignable, Category = "PT|Equip")
    FOnEquipRequested OnEquipRequested;

    // ── 일반 함수 ──
    UFUNCTION(BlueprintPure, Category = "PT|Equip")
    EItemType GetAllowedType() const { return AllowedType; }

protected:
    // ── 오버라이드 ──
    virtual bool CanAccept(const FInventorySlot& InSlot) const override;
    virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnInitialized() override;
    // ── 위젯 바인딩 ──
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* Img_Highlight;

    // ── 설정 ──
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Equip")
    EItemType AllowedType = EItemType::Weapon;

    // 수락 색
    UPROPERTY(EditAnywhere, Category = "PT|Equip")
    FLinearColor AcceptColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);

    // 거부 색
    UPROPERTY(EditAnywhere, Category = "PT|Equip")
    FLinearColor RejectColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

private:
    // ── 일반 함수 ──
    void UpdateHighlight(bool bIsOver, bool bCanAccept);
};
