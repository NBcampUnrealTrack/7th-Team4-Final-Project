#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "InputActionValue.h"
#include "Item/PTItemTypes.h"
#include "PTEquipmentComponent.h"
#include "PTPlayerCharacter.generated.h"

class UPTInventoryComponent;
class UPTEquipmentComponent;

UCLASS()
class PENTAGRAM_API APTPlayerCharacter : public APTBaseCharacter
{
    GENERATED_BODY()

public:
    APTPlayerCharacter();

    virtual void PossessedBy(AController* NewController) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnDeath() override;
    float GetTotalAttack() const;

    // F키 입력 시 상호작용 시도
    void TryInteract();

    void AddInvincibility();

    void RemoveInvincibility();

    // AnimNotify: 닷지 무적 구간 시작
    void OnDodgeInvincibleStart();

    // AnimNotify: 닷지 무적 구간 종료
    void OnDodgeInvincibleEnd();

    void OnChannelSkillActivateNotify();

    // 체력 재생
    void RegenHP();

    void RegenMP();

    // 상호작용 실제 처리 담당
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TryInteract(AActor* TargetActor);

    // 서버에서 스킬 호출
    UFUNCTION(Server, Reliable)
    void Server_UseSkill(FName SkillID, FVector_NetQuantize InTargetLoc, FVector_NetQuantizeNormal InAimDir, AActor* InTargetActor);
/*
    UFUNCTION(Server, Reliable)
    void Server_PlayAttackMontage(int32 MontageIndex);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAttackMontage(int32 MontageIndex);
*/
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDeathMontage();

    void RespawnAtLocation(const FVector& RespawnLocation);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ResetAfterRespawn();
/*
    UFUNCTION(Server, Reliable)
    void Server_StopAttack();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopAttack();
*/
    // [장비 컴포넌트] 무기 장착/해제 시 외형 업데이트 호출
    void UpdateWeaponVisual(const TSoftObjectPtr<UStaticMesh>& NewMeshAsset, const FItemData& ItemData = FItemData());

    void UpdateArmorVisual(EEquipSlotType SlotType, TSoftObjectPtr<USkeletalMesh> ArmorMesh);

    void UpdateHelmetVisual(const TSoftObjectPtr<UStaticMesh>& HelmetMeshAsset);

    void ApplyBuff(float BonusMultiplier, float Duration);

    void OnAtkBuffExpired();

    void ApplyWeaponAnimLayer(EWeaponType NewWeaponType);

    void EnterCombat();

    void StartCombatExitTimer();

    void OnCombatExitTimerExpired();

    void PrewarmWeaponAnimLayers();

    void AttachWeaponToSocket(bool bToHand);

    void OnCombatTransitionFinished();

    // 카메라를 가렸을 시 구조물 Alpha 처리
    UFUNCTION(BlueprintImplementableEvent, Category = "PT | CameraObscure")
    void OnStructureHidden(AActor* HidingActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "PT | CameraObscure")
    void OnStructureUnHidden(AActor* UnHiddenActor);

    UFUNCTION(BlueprintCallable, Category = "Equip")
    void EquipArmorChest(USkeletalMesh* NewArmorMesh);

    UFUNCTION()
    void OnRep_CurrentWeaponType();

    UFUNCTION()
    void Server_EnterCombat();

    FORCEINLINE UPTInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
    FORCEINLINE UPTEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

public:
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
/*
    UPROPERTY(VisibleAnywhere, Category = "Attack")
    int32 ComboIndex = 0;       // 현재 콤보 단계 (연속 공격 단계)


    UPROPERTY(VisibleAnywhere, Category = "Attack")
    bool bCanCombo = false;     // 콤보 입력 가능 여부

    UPROPERTY(VisibleAnywhere, Category = "Attack")
    bool bIsAttacking = false;  // 공격 중 여부
*/
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Skill")
    bool bIsUsingSkill = false;

    /*UPROPERTY(EditAnywhere, Category = "Attack")
    TArray<TObjectPtr<UAnimMontage>> AttackMontages; // 연속 공격 몽타주 배열
    */

    UPROPERTY(VisibleAnywhere, Category = "Dodge")
    bool bIsInvincible = false; // 무적 여부 (데미지 판정에서 참조)

    UPROPERTY(VisibleAnywhere, Category = "Dodge")
    bool bIsDodging = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsTransitioningToCombat = false;

    UPROPERTY(VisibleAnywhere, Category = "Dodge")
    float DodgeLaunchSpeed = 1200.f;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Buff")
    float AtkBuffBonus = 0.f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float NormalToCombatTransitionDuration = 0.35f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponType, VisibleAnywhere, Category = "Equip")
    EWeaponType CurrentWeaponType = EWeaponType::Hands;

    UPROPERTY()
    FItemData CurrentWeaponItemData;

    FName GetHolsterSocket(EWeaponType Type)const;

    FName GetHandSocket(EWeaponType Type) const;

    UPROPERTY()
    TSubclassOf<UAnimInstance> CurrentLinkedAnimLayerClass;

    UPROPERTY(EditAnywhere, Category = "Equip|AnimLayer")
    TSubclassOf<class UAnimInstance> HandAnimLayerClass;

    UPROPERTY(EditAnywhere, Category = "Equip|AnimLayer")
    TSubclassOf<UAnimInstance> SwordAnimLayerClass;

    UPROPERTY(EditAnywhere, Category = "Equip|AnimLayer")
    TSubclassOf<UAnimInstance> WandAnimLayerClass;

    UPROPERTY(EditAnywhere, Category = "Equip|AnimLayer")
    TSubclassOf<UAnimInstance> BowAnimLayerClass;

    UPROPERTY(VisibleAnywhere, Category = "Equip")
    TObjectPtr<USkeletalMeshComponent> ArmorChestMesh;

    UPROPERTY(EditAnywhere, Category = "Anim")
    TObjectPtr<UAnimMontage> DeathMontage;

    FTimerHandle HPRegenTimerHandle;

    FTimerHandle MPRegenTimerHandle;

    FTimerHandle BuffTimerHandle;

    FTimerHandle CombatExitTimerHandle;

    FTimerHandle CombatTransitionTimerHandle;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

    UPROPERTY(BlueprintAssignable)
    FOnPlayerDied OnPlayerDied;

    int32 InvincibleRefs = 0;

protected:
    // 무기 장착 스태틱 메시 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment | Visual")
    TObjectPtr<UStaticMeshComponent> WeaponMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment | Visual")
    TObjectPtr<UStaticMeshComponent> HelmetMeshComp;
    // 갑옷 장착 스태틱 메시 컴포넌트(추후)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment | Visual")
    TObjectPtr<UStaticMeshComponent> ChestMeshComp;

private:
    // 직전 프레임에 캐릭터를 가리고 있던 장애물 저장
    UPROPERTY()
    TObjectPtr<AActor> LastHidingActor = nullptr;


};
