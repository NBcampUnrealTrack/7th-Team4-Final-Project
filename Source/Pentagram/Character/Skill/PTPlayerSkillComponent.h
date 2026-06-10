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
    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 스킬 발동 시도
    virtual void TryActivateSkill(FName SkillID) override;

    // 닷지 발동 시도 (로컬 클라이언트에서 호출)
    void TryDodge();

    // ── RPC 함수 ─────────────────────────────────────────────────────────────

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

protected:
    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 쿨다운 종료 처리
    virtual void OnCooldownEnd(int32 SlotIndex) override;

public:
    // ── 멤버 변수 ────────────────────────────────────────────────────────────

    // DT에서 조회할 닷지 스킬 ID
    UPROPERTY(EditAnywhere, Category = "Dodge")
    FName DodgeSkillID = FName("Dodge");

    // 닷지 쿨다운 중 여부
    UPROPERTY(VisibleAnywhere, Category = "Dodge")
    bool bIsDodgeCooldown = false;

    // 닷지 쿨다운 타이머
    FTimerHandle DodgeCooldownTimer;

    // ── 델리게이트 (최하단) ──────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnSkillCooldownEnd, int32, SlotIndex);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTOnDodgeCooldownEnd);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnDodgeCooldownStart, float, Duration);

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownEnd OnSkillCooldownEnd;

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownStart OnSkillCooldownStart;

    UPROPERTY(BlueprintAssignable)
    FPTOnDodgeCooldownEnd OnDodgeCooldownEnd;

    UPROPERTY(BlueprintAssignable)
    FPTOnDodgeCooldownStart OnDodgeCooldownStart;
};
