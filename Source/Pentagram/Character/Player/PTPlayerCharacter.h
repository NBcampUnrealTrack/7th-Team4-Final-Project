#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "InputActionValue.h"
#include "PTPlayerCharacter.generated.h"

UCLASS()
class PENTAGRAM_API APTPlayerCharacter : public APTBaseCharacter
{
	GENERATED_BODY()

public:
    APTPlayerCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpringArm")
    TObjectPtr<class USpringArmComponent> SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<class UCameraComponent> CameraComp;

    /*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<USkillComponent> SkillComp;*/ //이동부터 우선 구현 후, 나중에 구현

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<class UInputAction> IA_Move;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> IA_Attack;

    UPROPERTY(EditAnywhere, Category = "Anim")
    TObjectPtr<UAnimMontage> DeathMontage;

#pragma region 일반 공격 관련

    UPROPERTY(VisibleAnywhere, Category = "Attack")
    int32 ComboIndex = 0;       // 현재 콤보 단계 (연속 공격 단계)
    UPROPERTY(VisibleAnywhere, Category = "Attack")
    bool bCanCombo = false;     // 콤보 입력 가능 여부
    UPROPERTY(VisibleAnywhere, Category = "Attack")
    bool bIsAttacking = false;   // 공격하는중인지
    UPROPERTY(EditAnywhere, Category = "Attack")
    TArray<TObjectPtr<UAnimMontage>> AttackMontages; // 연속 공격 몽타주 배열

    void PlayAttackMontage();
#pragma endregion

    void RegenHP();                  //체력 재생
    FTimerHandle HPRegenTimerHandle; // 체력 재생 타이머

    void MoveAction(const FInputActionValue& Value);
    void AttackAction(const FInputActionValue& Value);
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void OnDeath() override;                    //플레이어 죽음
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);  // 델리게이트
    UPROPERTY(BlueprintAssignable)
    FOnPlayerDied OnPlayerDied;

    virtual void PossessedBy(AController* NewController) override;
    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

};
