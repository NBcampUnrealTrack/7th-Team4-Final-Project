#include "PTInventorySlotWidget.h"
#include "Components/Image.h"
#include "Blueprint/DragDropOperation.h"
#include "PTItemTooltipWidget.h"
#include "PTEquipSlotWidget.h"

void UPTInventorySlotWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // 호버 툴팁
    ToolTipWidgetDelegate.BindDynamic(this, &UPTInventorySlotWidget::GetItemToolTip);

    RefreshIcon();
}

FReply UPTInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 빈 칸 제외
    if (IsEmpty()) return FReply::Unhandled();
    if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) return FReply::Unhandled();

    // 드래그 감지
    return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
}

FReply UPTInventorySlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (IsEmpty()) return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    const FKey Button = InMouseEvent.GetEffectingButton();

    // 우클릭 = 사용
    if (Button == EKeys::RightMouseButton)
    {
        OnUseRequested.Broadcast(SlotIndex);
        return FReply::Handled();
    }

    // 좌클릭 = 상점 판매, 선택
    if (Button == EKeys::LeftMouseButton)
    {
        OnClicked.Broadcast(SlotIndex);
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UPTInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (IsEmpty()) return;

    UDragDropOperation* Op = NewObject<UDragDropOperation>(this);
    if (!Op) return;

    // 출발 슬롯
    Op->Payload = this;
    Op->Pivot = EDragPivot::MouseDown;
    Op->DefaultDragVisual = CreateDragVisual();

    OutOperation = Op;
}

void UPTInventorySlotWidget::NativeOnDragCancelled(
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

    if (InOperation != nullptr && InOperation->Payload == this &&
        SlotIndex != INDEX_NONE && !IsEmpty())
    {
        OnFieldDropRequested.Broadcast(SlotIndex, InDragDropEvent.GetScreenSpacePosition());
    }
}

bool UPTInventorySlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

    // 드롭 대상 인식
    return true;
}

bool UPTInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UPTInventorySlotWidget* From = InOperation ? Cast<UPTInventorySlotWidget>(InOperation->Payload) : nullptr;
    if (!From) return false;
    if (From == this) return false;

    // 장비에서 온 것만 = 해제
    UPTEquipSlotWidget* EquipFrom = Cast<UPTEquipSlotWidget>(From);
    if (!EquipFrom) return false;

    // 임시 표시
    SetSlotData(EquipFrom->GetSlotData());
    EquipFrom->ClearSlot();

    // 해제 요청
    OnUnequipRequested.Broadcast(EquipFrom->GetAllowedType(), SlotIndex);
    return true;
}

void UPTInventorySlotWidget::SetSlotData(const FInventorySlot& InSlot)
{
    SlotData = InSlot;
    RefreshIcon();
}

void UPTInventorySlotWidget::ClearSlot()
{
    SlotData = FInventorySlot();
    RefreshIcon();
}

void UPTInventorySlotWidget::RefreshIcon()
{
    if (!Img_Icon) return;

    // 빈 칸 숨김
    if (SlotData.ItemIconAsset.IsNull())
    {
        Img_Icon->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    // 아이콘 그리기
    Img_Icon->SetBrushFromSoftTexture(SlotData.ItemIconAsset, false);
    Img_Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

UWidget* UPTInventorySlotWidget::CreateDragVisual()
{
    // WBP 우선
    if (DragVisualClass)
    {
        return CreateWidget<UUserWidget>(GetOwningPlayer(), DragVisualClass);
    }

    // 폴백 아이콘
    if (SlotData.ItemIconAsset.IsNull()) return nullptr;

    UImage* Ghost = NewObject<UImage>(this);
    if (!Ghost) return nullptr;

    Ghost->SetBrushFromSoftTexture(SlotData.ItemIconAsset, false);
    return Ghost;
}

UWidget* UPTInventorySlotWidget::GetItemToolTip()
{
    if (IsEmpty() || !ToolTipClass) return nullptr;

    UPTItemTooltipWidget* ToolTip = CreateWidget<UPTItemTooltipWidget>(GetOwningPlayer(), ToolTipClass);
    if (ToolTip)
    {
        ToolTip->SetItem(SlotData);
    }
    return ToolTip;
}
