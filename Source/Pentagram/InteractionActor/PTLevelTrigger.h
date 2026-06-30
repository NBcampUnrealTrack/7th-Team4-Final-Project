
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTLevelTrigger.generated.h"


UCLASS()
class PENTAGRAM_API APTLevelTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	APTLevelTrigger();

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TargetMapName Settings")
    FString TargetMapName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TargetActor Settings")
    FName TargetActorTag;

    UFUNCTION() 
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	virtual void Tick(float DeltaTime) override;

};
