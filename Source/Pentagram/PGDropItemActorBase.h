// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h" 
#include "GameFramework/Actor.h" 
#include "PGItemTypes.h" 
#include "PGDropItemActorBase.generated.h" 

class USphereComponent;
class UStaticMeshComponent; 

UCLASS() 
class PENTAGRAM_API APGDropItemActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APGDropItemActorBase();

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
