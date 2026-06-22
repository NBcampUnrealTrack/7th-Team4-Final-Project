#include "Character/NPC/PTShopNPCCharacter.h"

#include "Character/Player/PTPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

void APTShopNPCCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTShopNPCCharacter, ShopID);
    DOREPLIFETIME(APTShopNPCCharacter, ProductIDs);
    DOREPLIFETIME(APTShopNPCCharacter, ShopWidgetClass);
}

void APTShopNPCCharacter::Interact_Implementation(AActor* InteractorCharacter)
{
    Super::Interact_Implementation(InteractorCharacter);

    if (!HasAuthority() || InteractorCharacter == nullptr)
    {
        return;
    }

    APawn* InteractPawn = Cast<APawn>(InteractorCharacter);
    APlayerController* InteractPlayerController =
        InteractPawn != nullptr ? Cast<APlayerController>(InteractPawn->GetController()) : nullptr;

    OpenShop(InteractPlayerController);
}

void APTShopNPCCharacter::OpenShop(APlayerController* InteractPlayerController)
{
    if (InteractPlayerController == nullptr)
    {
        return;
    }

    APTPlayerController* PTPlayerController = Cast<APTPlayerController>(InteractPlayerController);
    if (PTPlayerController != nullptr)
    {
        PTPlayerController->Client_OpenShop(this, ShopWidgetClass);
    }

    OnShopOpened.Broadcast(InteractPlayerController);
}

void APTShopNPCCharacter::CloseShop(APlayerController* InteractPlayerController)
{
    if (InteractPlayerController == nullptr)
    {
        return;
    }

    OnShopClosed.Broadcast(InteractPlayerController);
}

FName APTShopNPCCharacter::GetShopID() const
{
    return ShopID;
}

const TArray<FName>& APTShopNPCCharacter::GetProductIDs() const
{
    return ProductIDs;
}
