#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "InputActionValue.h"
#include "PTPlayerCharacter.generated.h"

class UPTInventoryComponent; // 인벤토리 컴포넌트 유무
class UPTEquipmentComponent; // 장비창 컴포넌트 유무

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<class UPTSkillComponent> SkillComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPTInventoryComponent> InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPTEquipmentComponent> EquipmentComponent;

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

#pragma endregion

    // 플레이어가 F키를 눌렀을 때 호출할 메인 함수
    void TryInteract();

    // 상호작용의 실제 처리 담당
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TryInteract(AActor* TargetActor);

    //서버에서 스킬이 호출
    UFUNCTION(Server, Reliable)
    void Server_UseSkill(FName SkillID);

    void RegenHP();                  //체력 재생
    FTimerHandle HPRegenTimerHandle; // 체력 재생 타이머

    virtual void OnDeath() override;                    //플레이어 죽음
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);  // 델리게이트
    UPROPERTY(BlueprintAssignable)
    FOnPlayerDied OnPlayerDied;

    virtual void PossessedBy(AController* NewController) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    FORCEINLINE UPTInventoryComponent* GetInventoryComponent() const { return InventoryComponent; } // 인벤토리 컴포넌트 접근자
    FORCEINLINE UPTEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; } // 장비창 컴포넌트 접근자

};
