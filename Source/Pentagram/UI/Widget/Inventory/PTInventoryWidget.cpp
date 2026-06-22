#include "PTInventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PTInventorySlotWidget.h"
#include "Character/Player/PTInventoryComponent.h"

void UPTInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildSlots();
}

void UPTInventoryWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    RefreshAllSlots();
}

bool UPTInventoryWidget::NativeOnHandleBackAction()
{
    DeactivateWidget();
    return true;
}

void UPTInventoryWidget::BuildSlots()
{
    if (!InventoryGrid || !SlotClass) return;

    InventoryGrid->ClearChildren();
    SlotWidgets.Reset();

    for (int32 i = 0; i < SlotCount; ++i)
    {
        UPTInventorySlotWidget* InventorySlot = CreateWidget<UPTInventorySlotWidget>(this, SlotClass);
        if (!InventorySlot) continue;

        InventorySlot->SetSlotIndex(i);
        InventoryGrid->AddChildToUniformGrid(InventorySlot, i / Columns, i % Columns);
        SlotWidgets.Add(InventorySlot);
    }

}

void UPTInventoryWidget::RefreshAllSlots()
{
    UPTInventoryComponent* Inventory = ResolveInventoryComponent();

    if (!Inventory) return;

    const TArray<FInventorySlot>& Slots = Inventory->GetInventorySlots();

    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (!SlotWidgets[i]) continue;

        if (Slots.IsValidIndex(i) && !Slots[i].IsEmpty())
            SlotWidgets[i]->SetSlotData(Slots[i]);
        else
            SlotWidgets[i]->ClearSlot();
    }
}

UPTInventoryComponent* UPTInventoryWidget::ResolveInventoryComponent() const
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return nullptr;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return nullptr;

    return Pawn->FindComponentByClass<UPTInventoryComponent>();
}
