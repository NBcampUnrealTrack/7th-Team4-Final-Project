#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/PTCharacterRow.h"
#include "PTBaseCharacter.generated.h"

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

    UPROPERTY(VisibleAnywhere, Category = "Stats")
    float CurrentHP;   // 현재 체력
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    float MaxHP;       // 최대 체력
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    float BaseDef;     // 방어력
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    float BaseAtk;     // 공격력
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    float AttackSpeed; // 공격 속도
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    float MoveSpeed;   // 이동 속도

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
