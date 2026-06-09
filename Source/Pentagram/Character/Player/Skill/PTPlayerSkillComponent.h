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
    UFUNCTION(BlueprintCallable, Category = "Skill")
    virtual void TryActivateSkill(FName SkillID) override;

    // ── RPC 함수 ─────────────────────────────────────────────────────────────

    // 쿨다운 시작을 소유 클라이언트에게 통지
    UFUNCTION(Client, Reliable)
    void Client_NotifyCooldownStarted(int32 SlotIndex, float Duration);

protected:
    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 쿨다운 종료 처리
    virtual void OnCooldownEnd(int32 SlotIndex) override;

public:
    // ── 델리게이트 (최하단) ──────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnSkillCooldownEnd, int32, SlotIndex);

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownEnd OnSkillCooldownEnd;

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownStart OnSkillCooldownStart;
};
