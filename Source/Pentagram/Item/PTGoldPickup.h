#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTGoldPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class PENTAGRAM_API APTGoldPickup : public AActor
{
    GENERATED_BODY()

public:
    APTGoldPickup();

    void SetGoldAmount(int32 Amount) { GoldAmount = Amount; }

protected:
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "PT|Pickup")
    TObjectPtr<UStaticMeshComponent> GoldMesh;

    UPROPERTY(VisibleAnywhere, Category = "PT|Pickup")
    TObjectPtr<USphereComponent> CollisionSphere;

    int32 GoldAmount = 0;
    bool bPickedUp = false;
};
