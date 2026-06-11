#include "Character/NPC/PTShopNPCCharacter.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

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
