
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PTInteractableInterface.h"
#include "PTInteractionActor.generated.h"


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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ActorMesh;

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

};
