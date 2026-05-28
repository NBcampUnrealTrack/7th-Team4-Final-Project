#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/PTCharacterRow.h"
#include "PTBaseCharacter.generated.h"

class UPTInventoryComponent; // 인벤토리 컴포넌트 유무
class UPTEquipmentComponent; // 장비창 컴포넌트 유무 

UCLASS()
class PENTAGRAM_API APTBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APTBaseCharacter();

    //데미지 적용
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual float ApplyDamage(float DamageAmount, AActor* Attacker);

    //사망 처리
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void OnDeath();

    UPROPERTY(EditAnywhere, Category = "Data")
    FDataTableRowHandle CharacterDataHandle;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float CurrentHP;   // 현재 체력
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float MaxHP;       // 최대 체력
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float CurrentMP;   // 현재 마나
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float MaxMP;      // 최대 마나
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float BaseDef;     // 방어력
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float BaseAtk;     // 공격력
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float AttackSpeed; // 공격 속도
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float MoveSpeed;   // 이동 속도

protected:
	virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    FORCEINLINE UPTInventoryComponent* GetInventoryComponent() const { return InventoryComponent; } // Getter 함수 사용 : 블루프린트나 타 클래스에서 캐릭터의 인벤토리에 접근 가능
    FORCEINLINE UPTEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }  // Getter 함수 사용 : 블루프린트나 타 클래스에서 캐릭터의 장비창에 접근 가능

protected:
    void Input_MouseLeftClick(); // 탑뷰 마우스 좌클릭 상호작용 함수 선언 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true")) // 인벤토리 컴포넌트 추가
    UPTInventoryComponent* InventoryComponent; 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true")) // 장비창 컴포넌트 추가
    UPTEquipmentComponent* EquipmentComponent; 

}; 
