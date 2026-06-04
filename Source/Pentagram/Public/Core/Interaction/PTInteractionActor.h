// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Interface/PTInteractableInterface.h" // 인터페이스 가져오기 
#include "PTInteractionActor.generated.h"


// 액터가 어떤 종류의 상호작용을 할지 구별을 위한 열거형 
UENUM(BlueprintType)
enum class EInteractionType : uint8
{
    None			UMETA(DisplayName = "None"),
    TreasureBox		UMETA(DisplayName = "Treasure Box"),
    Door			UMETA(DisplayName = "Door"),
    RespawnPoint	UMETA(DisplayName = "Respawn Point"),
    Lever			UMETA(DisplayName = "Lever") 
};

UCLASS()
class PENTAGRAM_API APTInteractionActor : public AActor, public IPTInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APTInteractionActor();


protected: 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> InteractionActorMesh;

    // 에디터에서 액터의 종류 선택 가능 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    EInteractionType InteractionType;

    // 상자나 보상 아이템 지급 시 사용할 ID 데이터 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Data")
    FName RewardItemID;

    // 블루프린트 연출(파티클, 애니메이션)이벤트 - 이벤트그래프에서 노드 생성
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayInteractionEffects();


public:	
    // 인터페이스 상호작용 함수 오버라이드
    virtual void Interact_Implementation(AActor* InteractorCharacter) override;

};
