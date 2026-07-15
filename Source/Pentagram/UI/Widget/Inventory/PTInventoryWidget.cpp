#include "PTInventoryWidget.h"

#include "PTEquipPanelWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/Widget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PTInventorySlotWidget.h"
#include "Character/Player/PTEquipmentComponent.h"
#include "Character/Player/PTInventoryComponent.h"

#include "Character/Player/PTPlayerController.h"
#include "UI/Widget/Shop/PTShopWidget.h"

void UPTInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // CommonUI가 활성화 포커스를 게임 뷰포트로 되돌리지 않도록 합니다.
    SetIsFocusable(true);
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
    BindEquipmentChanged();
    RefreshAllSlots();
    RefreshEquipmentSlots();
}


void UPTInventoryWidget::NativeOnDeactivated()
{
    UnbindInventoryChanged();
    UnbindEquipmentChanged();
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

        InventorySlot->OnUseRequested.RemoveDynamic(this, &UPTInventoryWidget::HandleSlotUseRequested);
        InventorySlot->OnUseRequested.AddDynamic(this, &UPTInventoryWidget::HandleSlotUseRequested);

        InventorySlot->OnFieldDropRequested.RemoveDynamic(this, &UPTInventoryWidget::HandleFieldDropRequested);
        InventorySlot->OnFieldDropRequested.AddDynamic(this, &UPTInventoryWidget::HandleFieldDropRequested);

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

void UPTInventoryWidget::RefreshEquipmentSlots()
{
    if (EquipPanel == nullptr)
    {
        return;
    }

    EquipPanel->ClearSlot(EItemType::Weapon);
    EquipPanel->ClearSlot(EItemType::Chest);
    EquipPanel->ClearSlot(EItemType::Helmet);
    EquipPanel->ClearSlot(EItemType::Gloves);
    EquipPanel->ClearSlot(EItemType::Boots);

    UPTEquipmentComponent* Equipment = ResolveEquipmentComponent();
    if (Equipment == nullptr)
    {
        return;
    }

    for (const FEquipmentSlot& EquipmentSlot : Equipment->GetEquipmentSlots())
    {
        if (!EquipmentSlot.bIsEquipped || EquipmentSlot.MountedItem.Item_ID.IsNone())
        {
            continue;
        }

        FInventorySlot DisplaySlot;
        DisplaySlot.ItemData = EquipmentSlot.MountedItem;
        DisplaySlot.Quantity = 1;
        DisplaySlot.ItemIconAsset = EquipmentSlot.MountedItem.Item_Icon;
        DisplaySlot.ItemMeshAsset = EquipmentSlot.MountedItem.ItemMeshAsset;
        DisplaySlot.ArmorChestMeshAsset = EquipmentSlot.MountedItem.ArmorChestMeshAsset;
        EquipPanel->RefreshSlot(EquipmentSlot.MountedItem.Item_Type, DisplaySlot);
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

void UPTInventoryWidget::BindEquipmentChanged()
{
    UPTEquipmentComponent* Equipment = ResolveEquipmentComponent();
    if (Equipment == nullptr)
    {
        return;
    }

    if (BoundEquipmentComponent.IsValid() && BoundEquipmentComponent.Get() != Equipment)
    {
        UnbindEquipmentChanged();
    }

    Equipment->OnEquipmentChanged.RemoveDynamic(this, &UPTInventoryWidget::HandleEquipmentChanged);
    Equipment->OnEquipmentChanged.AddDynamic(this, &UPTInventoryWidget::HandleEquipmentChanged);
    BoundEquipmentComponent = Equipment;
}

void UPTInventoryWidget::UnbindEquipmentChanged()
{
    if (BoundEquipmentComponent.IsValid())
    {
        BoundEquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UPTInventoryWidget::HandleEquipmentChanged);
    }

    BoundEquipmentComponent.Reset();
}

void UPTInventoryWidget::HandleInventoryChanged()
{
    RefreshAllSlots();
}

void UPTInventoryWidget::HandleEquipmentChanged()
{
    RefreshEquipmentSlots();
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

UPTEquipmentComponent* UPTInventoryWidget::ResolveEquipmentComponent() const
{
    APlayerController* PlayerController = GetOwningPlayer();
    APawn* Pawn = PlayerController != nullptr ? PlayerController->GetPawn() : nullptr;
    return Pawn != nullptr ? Pawn->FindComponentByClass<UPTEquipmentComponent>() : nullptr;
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

void UPTInventoryWidget::HandleSlotUseRequested(int32 SlotIndex)
{
    // 상점에서 판매 대상 선택 모드일 땐 사용하지 않음 (안전장치)
    if (ShopWidgetForSell != nullptr) return;

    UPTInventoryComponent* Inventory = ResolveInventoryComponent();
    if (!Inventory) return;

    Inventory->UseItemAtSlot(SlotIndex);
}

void UPTInventoryWidget::HandleFieldDropRequested(int32 SlotIndex, FVector2D ScreenPosition)
{
    // 상점 판매 중이거나 인벤토리/장비 패널 안에 놓은 경우에는 필드 드랍으로 처리하지 않습니다.
    if (ShopWidgetForSell != nullptr || IsScreenPositionOverContent_Implementation(ScreenPosition))
    {
        return;
    }

    UPTInventoryComponent* Inventory = ResolveInventoryComponent();
    APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer());
    if (Inventory == nullptr || PlayerController == nullptr)
    {
        return;
    }

    const TArray<FInventorySlot>& Slots = Inventory->GetInventorySlots();
    if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
    {
        return;
    }

    const FInventorySlot& InventorySlot = Slots[SlotIndex];
    PlayerController->RequestDropInventoryItem(
        SlotIndex,
        InventorySlot.ItemData.Item_ID,
        InventorySlot.Quantity);
}

void UPTInventoryWidget::HandleCloseButtonClicked()
{
    DeactivateWidget();
}
