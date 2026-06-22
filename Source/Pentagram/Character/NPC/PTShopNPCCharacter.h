#pragma once

#include "CoreMinimal.h"
#include "Character/NPC/PTNPCCharacter.h"
#include "PTShopNPCCharacter.generated.h"

class APlayerController;
class UPTShopWidget;

UCLASS()
class PENTAGRAM_API APTShopNPCCharacter : public APTNPCCharacter
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Interact_Implementation(AActor* InteractorCharacter) override;

    void OpenShop(APlayerController* InteractPlayerController);
    void CloseShop(APlayerController* InteractPlayerController);

    FName GetShopID() const;
    const TArray<FName>& GetProductIDs() const;
    TSubclassOf<UPTShopWidget> GetShopWidgetClass() const { return ShopWidgetClass; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "PT|NPC|Shop")
    FName ShopID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "PT|NPC|Shop")
    TArray<FName> ProductIDs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "PT|NPC|Shop")
    TSubclassOf<UPTShopWidget> ShopWidgetClass;

public:
    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Shop")
    FPTNPCPlayerControllerDelegate OnShopOpened;

    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Shop")
    FPTNPCPlayerControllerDelegate OnShopClosed;
};
