#pragma once

#include "CoreMinimal.h"
#include "Character/NPC/PTNPCCharacter.h"
#include "PTShopNPCCharacter.generated.h"

class APlayerController;

UCLASS()
class PENTAGRAM_API APTShopNPCCharacter : public APTNPCCharacter
{
    GENERATED_BODY()

public:
    virtual void Interact_Implementation(AActor* InteractorCharacter) override;

    void OpenShop(APlayerController* InteractPlayerController);
    void CloseShop(APlayerController* InteractPlayerController);

    FName GetShopID() const;
    const TArray<FName>& GetProductIDs() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Shop")
    FName ShopID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Shop")
    TArray<FName> ProductIDs;

public:
    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Shop")
    FPTNPCPlayerControllerDelegate OnShopOpened;

    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Shop")
    FPTNPCPlayerControllerDelegate OnShopClosed;
};
