// Fill out your copyright notice in the Description page of Project Settings.


#include "PTShopWidget.h"

#include "Character/NPC/PTShopNPCCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Core/Subsystems/PTItemSubsystem.h"
#include "Item/PTItemTypes.h"
#include "PTShopSlotWidget.h"

void UPTShopWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Close != nullptr)
    {
        Btn_Close->OnClicked.AddUniqueDynamic(this, &UPTShopWidget::HandleCloseClicked);
    }
}

void UPTShopWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    BindPlayerStateDelegates();
    BuildProductList();
    RefreshGold();
}

void UPTShopWidget::NativeOnDeactivated()
{
    UnbindPlayerStateDelegates();

    if (APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer()))
    {
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

void UPTShopWidget::BuildProductList()
{
    DisplayedProductIDs.Empty();

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

        FInventorySlot ProductSlot;
        ProductSlot.ItemData = *ItemData;
        ProductSlot.Quantity = 1;

        ShopSlot->SetSlotIndex(SlotIndex);
        ShopSlot->SetShopData(ProductSlot, ItemData->BuyPrice);
        ShopSlot->OnBuyClicked.RemoveDynamic(this, &UPTShopWidget::HandleBuyRequested);
        ShopSlot->OnBuyClicked.AddDynamic(this, &UPTShopWidget::HandleBuyRequested);

        const int32 SafeColumnCount = FMath::Max(ColumnCount, 1);
        ProductGrid->AddChildToUniformGrid(
            ShopSlot,
            SlotIndex / SafeColumnCount,
            SlotIndex % SafeColumnCount);
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

void UPTShopWidget::HandleBuyRequested(int32 SlotIndex)
{
    if (TargetShopNPC == nullptr || !DisplayedProductIDs.IsValidIndex(SlotIndex))
    {
        return;
    }

    APTPlayerController* PlayerController = Cast<APTPlayerController>(GetOwningPlayer());
    if (PlayerController != nullptr)
    {
        PlayerController->ServerBuyItem(TargetShopNPC, DisplayedProductIDs[SlotIndex]);
    }
}

void UPTShopWidget::HandleGoldChanged(int64 NewAmount)
{
    RefreshGold();
}
