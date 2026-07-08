#include "PTInventoryWidget.h"

#include "PTEquipPanelWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/Widget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PTInventorySlotWidget.h"
#include "Character/Player/PTInventoryComponent.h"

#include "Character/Player/PTPlayerController.h"
#include "UI/Widget/Shop/PTShopWidget.h"

void UPTInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildSlots();

    if (EquipPanel)
    {
        EquipPanel->OnEquipRequested.AddUniqueDynamic(this, &UPTInventoryWidget::HandleEquipRequested);
    }

    for (UPTInventorySlotWidget* SlotWidget : SlotWidgets)
    {
        if (SlotWidget)
        {
            SlotWidget->OnUnequipRequested.AddUniqueDynamic(this, &UPTInventoryWidget::HandleUnequipRequested);
        }
    }

}

void UPTInventoryWidget::SetShopSellTarget(UPTShopWidget* InShopWidget)
{
    ShopWidgetForSell = InShopWidget;
}

void UPTInventoryWidget::ClearShopSellTarget()
{
    ShopWidgetForSell = nullptr;
}

bool UPTInventoryWidget::IsScreenPositionOverContent_Implementation(const FVector2D& ScreenPosition) const
{
    if (EquipPanel && EquipPanel->GetCachedGeometry().IsUnderLocation(ScreenPosition))
    {
        return true;
    }

    if (InventoryGrid && InventoryGrid->GetCachedGeometry().IsUnderLocation(ScreenPosition))
    {
        return true;
    }

    return false;
}

void UPTInventoryWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    if (UWidget* RootWidget = GetRootWidget())
    {
        RootWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }

    BindInventoryChanged();
    RefreshAllSlots();
}


void UPTInventoryWidget::NativeOnDeactivated()
{
    UnbindInventoryChanged();
    ClearShopSellTarget();

    if (APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PlayerController->RestoreGameplayInput();
    }

    Super::NativeOnDeactivated();
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
        InventorySlot->OnClicked.RemoveDynamic(this, &UPTInventoryWidget::HandleSlotClicked);
        InventorySlot->OnClicked.AddDynamic(this, &UPTInventoryWidget::HandleSlotClicked);
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

void UPTInventoryWidget::BindInventoryChanged()
{
    UPTInventoryComponent* Inventory = ResolveInventoryComponent();
    if (Inventory == nullptr)
    {
        return;
    }

    if (BoundInventoryComponent.IsValid() && BoundInventoryComponent.Get() != Inventory)
    {
        UnbindInventoryChanged();
    }

    Inventory->OnInventoryChanged.RemoveDynamic(this, &UPTInventoryWidget::HandleInventoryChanged);
    Inventory->OnInventoryChanged.AddDynamic(this, &UPTInventoryWidget::HandleInventoryChanged);
    BoundInventoryComponent = Inventory;
}

void UPTInventoryWidget::UnbindInventoryChanged()
{
    if (BoundInventoryComponent.IsValid())
    {
        BoundInventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UPTInventoryWidget::HandleInventoryChanged);
    }

    BoundInventoryComponent.Reset();
}

void UPTInventoryWidget::HandleInventoryChanged()
{
    RefreshAllSlots();
}

void UPTInventoryWidget::HandleSlotClicked(int32 SlotIndex)
{
    if (ShopWidgetForSell == nullptr || !SlotWidgets.IsValidIndex(SlotIndex) ||
        SlotWidgets[SlotIndex] == nullptr || SlotWidgets[SlotIndex]->IsEmpty())
    {
        return;
    }

    ShopWidgetForSell->SelectInventoryItemForSell(
        SlotIndex,
        SlotWidgets[SlotIndex]->GetSlotData());
}

UPTInventoryComponent* UPTInventoryWidget::ResolveInventoryComponent() const
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return nullptr;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return nullptr;

    return Pawn->FindComponentByClass<UPTInventoryComponent>();
}

void UPTInventoryWidget::HandleEquipRequested(int32 FromIndex, EItemType EquipType)
{
    APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer());
    if (!PC) return;

    PC->RequestEquipItem(FromIndex, EquipType);
}

void UPTInventoryWidget::HandleUnequipRequested(EItemType EquipType, int32 ToIndex)
{
    APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer());
    if (!PC) return;

    PC->RequestUnequipItem(EquipType, ToIndex);
}
