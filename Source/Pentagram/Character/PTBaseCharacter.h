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

    void PostInitializeComponents();

    // 데미지 적용
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual float ApplyDamage(float DamageAmount, AActor* Attacker);

    // 사망 처리
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void OnDeath();

    UFUNCTION()
    virtual void OnRep_CurrentHP();

    float GetAttackSpeed() const { return AttackSpeed; }

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    ECharacterType CharacterType;

    UPROPERTY(EditAnywhere, Category = "Data")
    FDataTableRowHandle CharacterDataHandle;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, VisibleAnywhere, Category = "Stats")
    float CurrentHP;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float MaxHP;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float CurrentMP;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float MaxMP;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float BaseDef;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float BaseAtk;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float AttackSpeed;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Stats")
    float MoveSpeed;
};
