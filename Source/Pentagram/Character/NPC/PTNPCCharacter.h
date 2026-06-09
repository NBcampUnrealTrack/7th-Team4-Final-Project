#pragma once

#include "CoreMinimal.h"
#include "Core/Interface/PTInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "PTNPCCharacter.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class USceneComponent;
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

    UFUNCTION(Server, Reliable)
    void ServerAcceptQuest(FName QuestID);

    UFUNCTION(Server, Reliable)
    void ServerRewardQuest(FName QuestID);

    FName GetNPCID() const;
    const TArray<FName>& GetQuestIDs() const;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC")
    FName NPCID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Quest")
    TArray<FName> QuestIDs;

private:
    static constexpr float InteractionRadius = 200.f;

public:
    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Interaction")
    FPTNPCPlayerControllerDelegate OnPlayerEnterRange;

    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Interaction")
    FPTNPCPlayerControllerDelegate OnPlayerExitRange;

    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Dialogue")
    FPTNPCPlayerControllerDelegate OnDialogueStarted;
};
