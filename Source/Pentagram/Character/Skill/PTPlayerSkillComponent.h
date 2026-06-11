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
    virtual void TryActivateSkill(FName SkillID) override;

    // 닷지 발동 시도 (로컬 클라이언트에서 호출)
    void TryDodge();

    // 쿨다운 시작을 소유 클라이언트에게 통지
    UFUNCTION(Client, Reliable)
    void Client_NotifyCooldownStarted(int32 SlotIndex, float Duration);

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

protected:
    // 쿨다운 종료 처리
    virtual void OnCooldownEnd(int32 SlotIndex) override;

public:
    // DT에서 조회할 닷지 스킬 ID
    UPROPERTY(EditAnywhere, Category = "Dodge")
    FName DodgeSkillID = FName("Dodge");


    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnSkillCooldownEnd, int32, SlotIndex);

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownEnd OnSkillCooldownEnd;

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownStart OnSkillCooldownStart;
};
