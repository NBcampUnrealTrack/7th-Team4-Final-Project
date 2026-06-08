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

    // [추가] 외부(캐릭터 등)에서 이 아이템의 데이터를 안전하게 읽어갈 수 있도록 Getter 함수를 제공합니다.
    // FORCEINLINE을 붙여서 성능을 높이고, const를 붙여 데이터 오염을 방지하는 정석적인 구조입니다.
    UFUNCTION(BlueprintCallable, Category = "Item Data")
    FORCEINLINE FItemData GetItemData() const { return InstanceItemData; }

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
    UPROPERTY(BlueprintReadOnly, Category = "Item Data")
    FItemData InstanceItemData;

    // 데이터 테이블로부터 초기화하는 함수
    void InitializeItemData();

};
