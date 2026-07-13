// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h" 
#include "GameFramework/Actor.h" 
#include "PTItemTypes.h" 
#include "PTDropItemActorBase.generated.h" 


class USphereComponent;
class UStaticMeshComponent; 

UCLASS() 
class PENTAGRAM_API APTDropItemActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APTDropItemActorBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // [추가] 외부(캐릭터 등)에서 이 아이템의 데이터를 안전하게 읽어갈 수 있도록 Getter 함수를 제공합니다.
    // FORCEINLINE을 붙여서 성능을 높이고, const를 붙여 데이터 오염을 방지하는 정석적인 구조입니다.
    UFUNCTION(BlueprintCallable, Category = "Item Data")
    FORCEINLINE FItemData GetItemData() const { return InstanceItemData; }

    UFUNCTION(BlueprintPure, Category = "Item Data")
    FORCEINLINE int32 GetDroppedQuantity() const { return DroppedQuantity; }

    /** 인벤토리에서 버린 아이템의 런타임 데이터와 수량을 서버에서 설정합니다. */
    void InitializeDroppedItem(const FItemData& InItemData, int32 InQuantity);

    /** 같은 드랍 액터를 두 플레이어가 동시에 줍는 것을 막습니다. */
    bool TryClaimPickup();
    void ReleasePickupClaim();

protected:
	virtual void BeginPlay() override;

    // 1단계 명세: 외형 및 충돌 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere; 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ItemMesh; 

    // 데이터 테이블에서 가져올 아이템 Row 이름 (에디터 배치용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FDataTableRowHandle ItemRowHandle; 

    // 실제 이 액터가 품고 있는 아이템 데이터
    UPROPERTY(ReplicatedUsing = OnRep_InstanceItemData, BlueprintReadOnly, Category = "Item Data")
    FItemData InstanceItemData;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Item Data")
    int32 DroppedQuantity = 1;

    // 데이터 테이블로부터 초기화하는 함수
    void InitializeItemData();

    UFUNCTION()
    void OnRep_InstanceItemData();

    void ApplyItemVisual();

private:
    bool bPickupClaimed = false;

};
