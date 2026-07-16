
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PTInteractableInterface.h"
#include "PTInteractionActor.generated.h"

class USphereComponent;
class UUserWidget;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EInteractType : uint8
{
    None			UMETA(DisplayName = "None"),
    Box		        UMETA(DisplayName = "Box"),
    Door			UMETA(DisplayName = "Door"),
    Lever			UMETA(DisplayName = "Lever"),
    RespawnPoint	UMETA(DisplayName = "Respawn Point")
};

UCLASS()
class PENTAGRAM_API APTInteractionActor : public AActor, public IPTInteractableInterface
{
	GENERATED_BODY()
	
public:	
	APTInteractionActor();

protected: 
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ActorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Interaction")
    TObjectPtr<USphereComponent> InteractionRangeSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Interaction")
    TObjectPtr<UWidgetComponent> RespawnSavedWidgetComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Interaction")
    TObjectPtr<UWidgetComponent> InteractionPromptWidgetComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Interaction",
        meta = (ClampMin = "1.0", UIMin = "1.0"))
    float InteractionRadius = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Interaction")
    FVector RespawnSavedWidgetRelativeLocation = FVector(0.f, 0.f, 150.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Interaction")
    FVector InteractionPromptRelativeLocation = FVector(0.f, 0.f, 150.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Interaction")
    TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    EInteractType InteractionType;
 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Data")
    FName RewardItemID;

    // 이미 상호작용 되었는지 여부를 서버에서 관리 (보물상자 중복 보상 / 문 중복 활성화 버그를 원천 차단)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Interaction")
    bool bIsInteracted = false;


    UFUNCTION(Client, Reliable)
    void Client_PlayLocalEffects();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayInteractionEffects();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayInteractionEffects();

public:	
    virtual void Interact_Implementation(AActor* InteractorCharacter) override;

private:
    void ShowInteractionPrompt();
    void HideInteractionPrompt();
    void ShowRespawnSavedLabel();
    void HideRespawnSavedLabel();
};
