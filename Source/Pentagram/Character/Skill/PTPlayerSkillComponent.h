#pragma once

#include "CoreMinimal.h"
#include "PTSkillComponent.h"
#include "UI/Data/PTDelegates.h"
#include "PTPlayerSkillComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTPlayerSkillComponent : public UPTSkillComponent
{
    GENERATED_BODY()

public:
    // 스킬 발동 시도
    virtual void TryActivateSkill(const FPTSkillActivationRequest& Request) override;

    // 닷지 발동 시도 (로컬 클라이언트에서 호출)
    void TryDodge();

    void TryBasicAttack();

    void PlayPendingAction();

    void ExecuteBasicAttack();

    // 쿨다운 시작을 소유 클라이언트에게 통지
    UFUNCTION(Client, Reliable)
    void Client_NotifyCooldownStarted(int32 SlotIndex, float Duration);

    UFUNCTION(Client, Reliable)
    void Client_NotifySkillSlotAssigned(int32 SlotIndex, FName SkillID);

    // 서버에서 닷지 처리
    UFUNCTION(Server, Reliable)
    void Server_Dodge();

    // 전체 클라이언트에 닷지 몽타주 재생
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDodgeMontage(UAnimMontage* Montage);

    // AnimNotify로부터 무적 ON/OFF를 서버에 전달
    UFUNCTION(Server, Reliable)
    void Server_SetInvincible(bool bInvincible);

    // 서버에서 닷지 종료 처리 (몽타주 종료 시 호출)
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDodgeEnded();

    UFUNCTION()
    void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SetPenetration(bool bEnable);

    //스킬 전방 대쉬
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_LaunchForSkill(FVector Velocity);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopMovementForSkill();

    UFUNCTION(Server, Reliable)
    void Server_BasicAttack(int32 InComboIndex);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayBasicAttackMontage(int32 InComboIndex);

    UFUNCTION(Server, Reliable)
    void Server_StopBasicAttack();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopBasicAttack();

protected:
    // 쿨다운 종료 처리
    virtual void OnCooldownEnd(int32 SlotIndex) override;

public:
    // DT에서 조회할 닷지 스킬 ID
    UPROPERTY(EditAnywhere, Category = "Dodge")
    FName DodgeSkillID = FName("Dodge");

    UPROPERTY(EditAnywhere, Category = "BasicAttack")
    FName BasicAttackSkillID = FName("BasicAttack");

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnSkillCooldownEnd, int32, SlotIndex);

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownEnd OnSkillCooldownEnd;

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownStart OnSkillCooldownStart;

    UPROPERTY(VisibleAnywhere, Category = "Attack")
    int32 ComboIndex = 0;       // 현재 콤보 단계 (연속 공격 단계)

    UPROPERTY(VisibleAnywhere, Category = "Attack")
    bool bCanCombo = false;     // 콤보 입력 가능 여부

    UPROPERTY(VisibleAnywhere, Category = "Attack")
    bool bIsAttacking = false;  // 공격 중 여부
    
    UPROPERTY(BlueprintAssignable)
    FPTOnSkillSlotAssigned OnSkillSlotAssigned;


    bool bPendingBasicAttack = false;

    bool bPendingSkill = false;

    FPTSkillActivationRequest PendingSkillRequest;
};
