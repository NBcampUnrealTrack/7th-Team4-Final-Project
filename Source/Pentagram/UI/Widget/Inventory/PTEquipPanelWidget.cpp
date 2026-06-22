#include "PTEquipPanelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "PTEquipSlotWidget.h"

void UPTEquipPanelWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    CollectSlots();
}

void UPTEquipPanelWidget::CollectSlots()
{
    if (!WidgetTree) return;

    // 슬롯 수집/바인딩
    WidgetTree->ForEachWidget([this](UWidget* Widget)
    {
        UPTEquipSlotWidget* Slot = Cast<UPTEquipSlotWidget>(Widget);
        if (!Slot) return;

        SlotMap.Add(Slot->GetAllowedType(), Slot);
        Slot->OnEquipRequested.AddUniqueDynamic(this, &UPTEquipPanelWidget::HandleEquipRequested);
    });
}

void UPTEquipPanelWidget::HandleEquipRequested(int32 FromIndex, EItemType EquipType)
{
    // 상위로 전달
    OnEquipRequested.Broadcast(FromIndex, EquipType);
}

void UPTEquipPanelWidget::RefreshSlot(EItemType Type, const FInventorySlot& InSlot)
{
    UPTEquipSlotWidget** Found = SlotMap.Find(Type);
    if (!Found || !*Found) return;

    (*Found)->SetSlotData(InSlot);
}

void UPTEquipPanelWidget::ClearSlot(EItemType Type)
{
    UPTEquipSlotWidget** Found = SlotMap.Find(Type);
    if (!Found || !*Found) return;

    (*Found)->ClearSlot();
}
