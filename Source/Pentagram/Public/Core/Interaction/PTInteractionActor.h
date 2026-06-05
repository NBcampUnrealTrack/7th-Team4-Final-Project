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
    Lever			UMETA(DisplayName = "Lever"),
    RespawnPoint	UMETA(DisplayName = "Respawn Point")
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

    // 에디터에서 액터의 종류 선택 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    EInteractionType InteractionType;

    // 상자나 보상 아이템 지급 시 사용할 ID 데이터 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Data")
    FName RewardItemID;

    // 이미 상호작용이 되었는지 여부를 서버에서 관리 (보물상자 중복 보상 및 문 중복 활성화 버그를 원천 차단)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Interaction")
    bool bIsInteracted = false;


    // 상호작용한 클라이언트 본인의 화면에서만 연출 실행 클라이언트 RPC
    UFUNCTION(Client, Reliable)
    void Client_PlayLocalEffects();

    // 서버가 실행하면 접속 중인 모든 클라이언트들에게 연출 실행 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayInteractionEffects();

    // 블루프린트 연출(파티클, 애니메이션)이벤트 - 이벤트그래프에서 노드 생성
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayInteractionEffects();


public:	
    // 인터페이스 상호작용 함수 오버라이드
    virtual void Interact_Implementation(AActor* InteractorCharacter) override;

};
