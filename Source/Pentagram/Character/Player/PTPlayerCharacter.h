#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "InputActionValue.h"
#include "PTPlayerCharacter.generated.h"

class UPTInventoryComponent;
class UPTEquipmentComponent;

UCLASS()
class PENTAGRAM_API APTPlayerCharacter : public APTBaseCharacter
{
    GENERATED_BODY()

public:
    APTPlayerCharacter();

    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void PossessedBy(AController* NewController) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnDeath() override;

    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // F키 입력 시 상호작용 시도
    void TryInteract();

    // 체력 재생
    void RegenHP();

    // ── RPC 함수 ─────────────────────────────────────────────────────────────

    // 상호작용 실제 처리 담당
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TryInteract(AActor* TargetActor);

    // 서버에서 스킬 호출
    UFUNCTION(Server, Reliable)
    void Server_UseSkill(FName SkillID);

    UFUNCTION(Server, Reliable)
    void Server_PlayAttackMontage(int32 MontageIndex);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAttackMontage(int32 MontageIndex);

    UFUNCTION(Server, Reliable)
    void Server_Dodge();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDodgeMontage();

    // ── Getter 함수 ──────────────────────────────────────────────────────────

    FORCEINLINE UPTInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
    FORCEINLINE UPTEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

public:
    // ── 멤버 변수 ────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpringArm")
    TObjectPtr<class USpringArmComponent> SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<class UCameraComponent> CameraComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<class UPTPlayerSkillComponent> SkillComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UPTInventoryComponent> InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TObjectPtr<UPTEquipmentComponent> EquipmentComponent;

    // ── 일반 공격 관련 ───────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, Category = "Attack")
    int32 ComboIndex = 0;       // 현재 콤보 단계 (연속 공격 단계)

    UPROPERTY(VisibleAnywhere, Category = "Attack")
    bool bCanCombo = false;     // 콤보 입력 가능 여부

    UPROPERTY(VisibleAnywhere, Category = "Attack")
    bool bIsAttacking = false;  // 공격 중 여부

    UPROPERTY(EditAnywhere, Category = "Attack")
    TArray<TObjectPtr<UAnimMontage>> AttackMontages; // 연속 공격 몽타주 배열

    // ── 닷지 관련 ────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, Category = "Dodge")
    bool bIsInvincible = false; // 무적 여부 (데미지 판정에서 참조)

    UPROPERTY(EditAnywhere, Category = "Anim")
    TObjectPtr<UAnimMontage> DodgeMontage;

    // ── 애니메이션 ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, Category = "Anim")
    TObjectPtr<UAnimMontage> DeathMontage;

    // ── 체력 재생 타이머 ─────────────────────────────────────────────────────

    FTimerHandle HPRegenTimerHandle;

    // ── 델리게이트 (최하단) ──────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

    UPROPERTY(BlueprintAssignable)
    FOnPlayerDied OnPlayerDied;
};
