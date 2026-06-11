#pragma once

#include "CoreMinimal.h"
#include "Interface/PTInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "PTNPCCharacter.generated.h"

class USphereComponent;
class UBoxComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UUserWidget;
class UWidgetComponent;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTNPCPlayerControllerDelegate, APlayerController*, PlayerController);

UCLASS()
class PENTAGRAM_API APTNPCCharacter : public AActor, public IPTInteractableInterface
{
    GENERATED_BODY()

public:
    APTNPCCharacter();

    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* InteractorCharacter) override;

    FName GetNPCID() const;

protected:
    UFUNCTION()
    void OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<USceneComponent> SceneRootComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<USphereComponent> InteractionRangeSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<UBoxComponent> InteractionCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<UWidgetComponent> InteractionPromptWidgetComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC")
    FName NPCID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Interaction")
    float InteractionRadius = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Interaction")
    FVector InteractionCollisionExtent = FVector(50.f, 50.f, 100.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Interaction")
    TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Interaction")
    FVector InteractionPromptRelativeLocation = FVector(0.f, 80.f, 140.f);

private:
    void ShowInteractionPrompt(APlayerController* PlayerController);
    void HideInteractionPrompt(APlayerController* PlayerController);

public:
    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Interaction")
    FPTNPCPlayerControllerDelegate OnPlayerEnterRange;

    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Interaction")
    FPTNPCPlayerControllerDelegate OnPlayerExitRange;

    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Dialogue")
    FPTNPCPlayerControllerDelegate OnDialogueStarted;
};
