// Fill out your copyright notice in the Description page of Project Settings.


#include "PTShopWidget.h"

#include "Character/NPC/PTShopNPCCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Core/Subsystems/PTItemSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Item/PTItemTypes.h"
#include "UI/Manage/PTUIManagerSubsystem.h"
#include "UI/Widget/Item/PTItemInfoPanel.h"
#include "PTShopSlotWidget.h"

void UPTShopWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Close != nullptr)
    {
        Btn_Close->OnClicked.AddUniqueDynamic(this, &UPTShopWidget::HandleCloseClicked);
    }

    if (Btn_Buy != nullptr)
    {
        Btn_Buy->OnClicked.AddUniqueDynamic(this, &UPTShopWidget::HandleBuyClicked);
    }

    ResetProductSelection();
}

void UPTShopWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    if (APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PlayerController->SetGameplayInputBlockedByUI(true);
        PlayerController->SetShowMouseCursor(true);
    }

    BindPlayerStateDelegates();
    BuildProductList();
    RefreshGold();
}

void UPTShopWidget::NativeOnDeactivated()
{
    UnbindPlayerStateDelegates();

    if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
    {
        if (UPTUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPTUIManagerSubsystem>())
        {
            UIManager->CloseShopInventory();
        }
    }

    if (APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PlayerController->SetGameplayInputBlockedByUI(false);
        PlayerController->RestoreGameplayInput();
    }

    Super::NativeOnDeactivated();
}

bool UPTShopWidget::NativeOnHandleBackAction()
{
    bIsBackHandler = true;
    DeactivateWidget(); // 닫기
    return true;
}

void UPTShopWidget::HandleCloseClicked()
{
    DeactivateWidget();
}

void UPTShopWidget::SetupShop(APTShopNPCCharacter* InShopNPC)
{
    TargetShopNPC = InShopNPC;

    BindPlayerStateDelegates();
    BuildProductList();
    RefreshGold();
}

void UPTShopWidget::SelectInventoryItemForSell(int32 InventorySlotIndex, const FInventorySlot& SlotData)
{
    SelectedSlotIndex = INDEX_NONE;
    SelectedSellInventorySlotIndex = INDEX_NONE;
    SelectedSellItemID = NAME_None;
    HoveredSlotIndex = INDEX_NONE;

    if (ItemInfoPanel != nullptr)
    {
        ItemInfoPanel->ClearItemData();
    }

    const bool bCanSell = !SlotData.IsEmpty() && SlotData.ItemData.CanSell();
    const int32 SellPrice = bCanSell ? SlotData.ItemData.GetSellPrice() : 0;

    if (Txt_SelectedPrice != nullptr)
    {
        if (bCanSell && SellPrice > 0)
        {
            const FText PriceText = FText::Format(
                NSLOCTEXT("PTShop", "SelectedSellPriceFormat", "{0} G"),
                FText::AsNumber(SellPrice));
            Txt_SelectedPrice->SetText(PriceText);
        }
        else
        {
            Txt_SelectedPrice->SetText(NSLOCTEXT("PTShop", "CannotSellItem", "Cannot Sell"));
        }
    }

    SetActionButtonText(NSLOCTEXT("PTShop", "SellButtonLabel", "Sell"));

    if (Btn_Buy != nullptr)
    {
        Btn_Buy->SetIsEnabled(bCanSell && SellPrice > 0);
    }

    if (bCanSell && SellPrice > 0)
    {
        SelectedSellInventorySlotIndex = InventorySlotIndex;
        SelectedSellItemID = SlotData.ItemData.Item_ID;
    }
}

void UPTShopWidget::BuildProductList()
{
    DisplayedProductIDs.Empty();
    DisplayedProductPrices.Empty();
    DisplayedProductData.Empty();
    DisplayedProductSlots.Empty();
    ResetProductSelection();

    if (ProductGrid == nullptr)
    {
        return;
    }

    ProductGrid->ClearChildren();

    if (TargetShopNPC == nullptr || ShopSlotWidgetClass == nullptr)
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UPTItemSubsystem* ItemSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTItemSubsystem>() : nullptr;
    if (ItemSubsystem == nullptr)
    {
        return;
    }

    for (FName ProductID : TargetShopNPC->GetProductIDs())
    {
        const FItemData* ItemData = ItemSubsystem->GetItemData(ProductID);
        if (ItemData == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shop] Product data was not found: %s"), *ProductID.ToString());
            continue;
        }

        UPTShopSlotWidget* ShopSlot = CreateWidget<UPTShopSlotWidget>(
            GetOwningPlayer(),
            ShopSlotWidgetClass);
        if (ShopSlot == nullptr)
        {
            continue;
        }

        const int32 SlotIndex = DisplayedProductIDs.Add(ProductID);
        DisplayedProductPrices.Add(ItemData->BuyPrice);
        DisplayedProductData.Add(*ItemData);
        DisplayedProductSlots.Add(ShopSlot);

        FInventorySlot ProductSlot;
        ProductSlot.ItemData = *ItemData;
        ProductSlot.Quantity = 1;

        ShopSlot->SetSlotIndex(SlotIndex);
        ShopSlot->SetShopData(ProductSlot, ItemData->BuyPrice);
        ShopSlot->OnSelected.RemoveDynamic(this, &UPTShopWidget::HandleProductSelected);
        ShopSlot->OnSelected.AddDynamic(this, &UPTShopWidget::HandleProductSelected);
        ShopSlot->OnHovered.RemoveDynamic(this, &UPTShopWidget::HandleProductHovered);
        ShopSlot->OnHovered.AddDynamic(this, &UPTShopWidget::HandleProductHovered);
        ShopSlot->OnUnhovered.RemoveDynamic(this, &UPTShopWidget::HandleProductUnhovered);
        ShopSlot->OnUnhovered.AddDynamic(this, &UPTShopWidget::HandleProductUnhovered);

        const int32 SafeColumnCount = FMath::Max(ColumnCount, 1);
        ProductGrid->AddChildToUniformGrid(
            ShopSlot,
            SlotIndex / SafeColumnCount,
            SlotIndex % SafeColumnCount);
    }
}

void UPTShopWidget::ResetProductSelection()
{
    SelectedSlotIndex = INDEX_NONE;
    SelectedSellInventorySlotIndex = INDEX_NONE;
    SelectedSellItemID = NAME_None;
    HoveredSlotIndex = INDEX_NONE;

    if (Txt_SelectedPrice != nullptr)
    {
        Txt_SelectedPrice->SetText(FText::GetEmpty());
    }

    if (Btn_Buy != nullptr)
    {
        Btn_Buy->SetIsEnabled(false);
    }

    SetActionButtonText(NSLOCTEXT("PTShop", "BuyButtonLabel", "Buy"));

    if (ItemInfoPanel != nullptr)
    {
        ItemInfoPanel->ClearItemData();
    }
}

void UPTShopWidget::SetActionButtonText(const FText& InText)
{
    if (Txt_BuyLabel != nullptr)
    {
        Txt_BuyLabel->SetText(InText);
    }
}

void UPTShopWidget::RefreshGold()
{
    if (Txt_CurrentGold == nullptr || GetOwningPlayer() == nullptr)
    {
        return;
    }

    const APTBasePlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<APTBasePlayerState>();
    const int32 CurrentGold = PlayerState != nullptr ? PlayerState->CurrentGold : 0;
    Txt_CurrentGold->SetText(FText::AsNumber(CurrentGold));
}

void UPTShopWidget::BindPlayerStateDelegates()
{
    if (GetOwningPlayer() == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<APTBasePlayerState>();
    if (PlayerState == nullptr)
    {
        return;
    }

    PlayerState->OnGoldChanged.RemoveDynamic(this, &UPTShopWidget::HandleGoldChanged);
    PlayerState->OnGoldChanged.AddDynamic(this, &UPTShopWidget::HandleGoldChanged);
}

void UPTShopWidget::UnbindPlayerStateDelegates()
{
    if (GetOwningPlayer() == nullptr)
    {
        return;
    }

    APTBasePlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<APTBasePlayerState>();
    if (PlayerState != nullptr)
    {
        PlayerState->OnGoldChanged.RemoveDynamic(this, &UPTShopWidget::HandleGoldChanged);
    }
}

void UPTShopWidget::HandleProductSelected(int32 SlotIndex)
{
    if (!DisplayedProductIDs.IsValidIndex(SlotIndex) ||
        !DisplayedProductPrices.IsValidIndex(SlotIndex) ||
        !DisplayedProductData.IsValidIndex(SlotIndex))
    {
        ResetProductSelection();
        return;
    }

    SelectedSlotIndex = SlotIndex;
    SelectedSellInventorySlotIndex = INDEX_NONE;
    SelectedSellItemID = NAME_None;
    HoveredSlotIndex = INDEX_NONE;

    if (ItemInfoPanel != nullptr)
    {
        ItemInfoPanel->ClearItemData();
    }

    if (Txt_SelectedPrice != nullptr)
    {
        const FText PriceText = FText::Format(
            NSLOCTEXT("PTShop", "SelectedPriceFormat", "{0} G"),
            FText::AsNumber(DisplayedProductPrices[SelectedSlotIndex]));
        Txt_SelectedPrice->SetText(PriceText);
    }

    if (Btn_Buy != nullptr)
    {
        Btn_Buy->SetIsEnabled(true);
    }

    SetActionButtonText(NSLOCTEXT("PTShop", "BuyButtonLabel", "Buy"));

}

void UPTShopWidget::HandleProductHovered(int32 SlotIndex)
{
    if (ItemInfoPanel == nullptr ||
        !DisplayedProductData.IsValidIndex(SlotIndex) ||
        !DisplayedProductSlots.IsValidIndex(SlotIndex))
    {
        return;
    }

    HoveredSlotIndex = SlotIndex;
    ItemInfoPanel->SetItemData(DisplayedProductData[HoveredSlotIndex]);
    PositionItemInfoPanel(HoveredSlotIndex);
}

void UPTShopWidget::HandleProductUnhovered(int32 SlotIndex)
{
    if (HoveredSlotIndex != SlotIndex)
    {
        return;
    }

    HoveredSlotIndex = INDEX_NONE;

    if (ItemInfoPanel != nullptr)
    {
        ItemInfoPanel->ClearItemData();
    }
}

void UPTShopWidget::PositionItemInfoPanel(int32 SlotIndex)
{
    if (ItemInfoPanel == nullptr || !DisplayedProductSlots.IsValidIndex(SlotIndex))
    {
        return;
    }

    UPTShopSlotWidget* SelectedSlot = DisplayedProductSlots[SlotIndex];
    UCanvasPanelSlot* InfoCanvasSlot = Cast<UCanvasPanelSlot>(ItemInfoPanel->Slot);
    UCanvasPanel* ParentCanvas = Cast<UCanvasPanel>(ItemInfoPanel->GetParent());
    if (SelectedSlot == nullptr || InfoCanvasSlot == nullptr || ParentCanvas == nullptr)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[Shop] ItemInfoPanel must be a direct child of the shop's Canvas Panel."));
        return;
    }

    ItemInfoPanel->ForceLayoutPrepass();

    const FGeometry& CanvasGeometry = ParentCanvas->GetCachedGeometry();
    const FGeometry& SlotGeometry = SelectedSlot->GetCachedGeometry();
    const FVector2D CanvasSize = CanvasGeometry.GetLocalSize();

    const FVector2D SlotTopLeft = CanvasGeometry.AbsoluteToLocal(
        SlotGeometry.LocalToAbsolute(FVector2D::ZeroVector));
    const FVector2D SlotBottomRight = CanvasGeometry.AbsoluteToLocal(
        SlotGeometry.LocalToAbsolute(SlotGeometry.GetLocalSize()));
    const FVector2D SlotSize = SlotBottomRight - SlotTopLeft;

    FVector2D PanelSize = InfoCanvasSlot->GetSize();
    if (PanelSize.X <= 0.0f || PanelSize.Y <= 0.0f)
    {
        PanelSize = ItemInfoPanel->GetDesiredSize();
    }

    const float LeftX = SlotTopLeft.X - ItemInfoPanelGap - PanelSize.X;
    const float RightX = SlotTopLeft.X + SlotSize.X + ItemInfoPanelGap;
    const bool bCanFitLeft = LeftX >= 0.0f;
    const bool bCanFitRight = RightX + PanelSize.X <= CanvasSize.X;

    float PanelX = RightX;
    if (bPreferItemInfoPanelOnLeft && bCanFitLeft)
    {
        PanelX = LeftX;
    }
    else if (bCanFitRight)
    {
        PanelX = RightX;
    }
    else if (bCanFitLeft)
    {
        PanelX = LeftX;
    }
    else
    {
        const float LeftSpace = SlotTopLeft.X;
        const float RightSpace = CanvasSize.X - SlotBottomRight.X;
        PanelX = LeftSpace >= RightSpace ? LeftX : RightX;
    }

    const float PanelY = SlotTopLeft.Y + (SlotSize.Y - PanelSize.Y) * 0.5f;
    const FVector2D MaxPosition(
        FMath::Max(CanvasSize.X - PanelSize.X, 0.0f),
        FMath::Max(CanvasSize.Y - PanelSize.Y, 0.0f));

    InfoCanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
    InfoCanvasSlot->SetAlignment(FVector2D::ZeroVector);
    InfoCanvasSlot->SetPosition(FVector2D(
        FMath::Clamp(PanelX, 0.0f, MaxPosition.X),
        FMath::Clamp(PanelY, 0.0f, MaxPosition.Y)));
    InfoCanvasSlot->SetZOrder(100);
}

void UPTShopWidget::HandleBuyClicked()
{
    if (TargetShopNPC == nullptr)
    {
        ResetProductSelection();
        return;
    }

    APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer());
    if (PlayerController == nullptr)
    {
        return;
    }

    if (SelectedSellInventorySlotIndex != INDEX_NONE && !SelectedSellItemID.IsNone())
    {
        PlayerController->ServerSellItem(
            TargetShopNPC,
            SelectedSellInventorySlotIndex,
            SelectedSellItemID,
            1);
        ResetProductSelection();
        return;
    }

    if (!DisplayedProductIDs.IsValidIndex(SelectedSlotIndex))
    {
        ResetProductSelection();
        return;
    }

    if (PlayerController != nullptr)
    {
        PlayerController->ServerBuyItem(
            TargetShopNPC,
            DisplayedProductIDs[SelectedSlotIndex]);
    }
}

void UPTShopWidget::HandleGoldChanged(int64 NewAmount)
{
    RefreshGold();
}
