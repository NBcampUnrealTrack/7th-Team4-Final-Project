// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTItemTypes.h"
#include "PTInventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PENTAGRAM_API UPTInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPTInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:	
    // 아이템 추가 시도 함수 (성공 시 true, 가방이 가득 차면 false)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItem(const FItemData& NewItemData, int32 Count = 1);

    void PrintInventoryLog(); // 디버깅용 : 현재 인벤토리 상태를 로그창에 예쁘게 출력

    // 블루프린트나 캐릭터에서 호출할 물약 사용 함수
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UsePotion(int32 SlotIndex);


protected:
    // 가방 크기 총 30칸 
    const int32 MaxSlotCount = 30;

    // 인벤토리 실제 데이터를 담는 배열
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> InventorySlots; 

private:
    // 회복을 주기적으로 실행할 타이머 핸들 및 카운터
    FTimerHandle PotionTimerHandle;
    int32 PotionTickCount = 0; 

    // 실제 캐릭터를 찾아서 피를 채워줄 내부 틱 함수
    void ExecutePotionHealing(); 

private:
    // 소비 아이템용: 동일한 아이템 ID를 가진 슬롯의 인덱스를 반환 (없으면 INDEX_NONE)
    int32 FindSameItemSlot(const FName& ItemID) const;

    // 빈 슬롯의 인덱스를 반환 (없으면 INDEX_NONE)
    int32 FindEmptySlot() const;
    


		
};
