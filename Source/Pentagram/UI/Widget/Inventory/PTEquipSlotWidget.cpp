#include "PTEquipSlotWidget.h"

#include "Components/Image.h"
#include "Blueprint/DragDropOperation.h"

bool UPTEquipSlotWidget::CanAccept(const FInventorySlot& InSlot) const
{
    if (InSlot.IsEmpty()) return false;

    // ↓ 구조에 맞게 ↓
    const EItemCategory Category = InSlot.ItemData.Item_Category;
    const EItemType Type = InSlot.ItemData.Item_Type;
    // ↑ 구조에 맞게 ↑

    if (Category != EItemCategory::Equipment) return false;
    if (Type != AllowedType) return false;

    return true;
}
void UPTEquipSlotWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // 하이라이트 끄기
    UpdateHighlight(false, false);
}

bool UPTEquipSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

    // 드롭 대상 인식
    return true;
}

void UPTEquipSlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

    UPTInventorySlotWidget* From = InOperation ? Cast<UPTInventorySlotWidget>(InOperation->Payload) : nullptr;
    const bool bCan = From && CanAccept(From->GetSlotData());

    UpdateHighlight(true, bCan);
}

void UPTEquipSlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragLeave(InDragDropEvent, InOperation);

    UpdateHighlight(false, false);
}

bool UPTEquipSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UpdateHighlight(false, false);

    UPTInventorySlotWidget* From = InOperation ? Cast<UPTInventorySlotWidget>(InOperation->Payload) : nullptr;
    if (!From) return false;
    if (From == this) return false;
    if (!CanAccept(From->GetSlotData())) return false;

    SetSlotData(From->GetSlotData());
    // 장착 요청
    OnEquipRequested.Broadcast(From->GetSlotIndex(), AllowedType);

    UE_LOG(LogTemp, Warning, TEXT("OnDrop From=%d"), From ? From->GetSlotIndex() : -1);
    return true;
}

void UPTEquipSlotWidget::UpdateHighlight(bool bIsOver, bool bCanAccept)
{
    if (!Img_Highlight) return;

    // 안 올라옴 숨김
    if (!bIsOver)
    {
        Img_Highlight->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    // 색 적용
    Img_Highlight->SetColorAndOpacity(bCanAccept ? AcceptColor : RejectColor);
    Img_Highlight->SetVisibility(ESlateVisibility::HitTestInvisible);
}
