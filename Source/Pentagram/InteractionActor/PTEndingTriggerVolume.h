#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTEndingTriggerVolume.generated.h"

class UBoxComponent;
class APTPlayerCharacter;

UCLASS()
class PENTAGRAM_API APTEndingTriggerVolume : public AActor
{
    GENERATED_BODY()

public:
    APTEndingTriggerVolume();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void CheckClearCondition();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> TriggerVolume;

private:
    UPROPERTY()
    TSet<TObjectPtr<APTPlayerCharacter>> PlayersInVolume;

    bool bTriggered = false;
};
