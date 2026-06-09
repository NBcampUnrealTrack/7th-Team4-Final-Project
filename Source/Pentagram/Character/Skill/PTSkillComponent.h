#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTSkillRow.h"
#include "PTSkillComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTSkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPTSkillComponent();

    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 스킬 발동 시도
    UFUNCTION(BlueprintCallable, Category = "Skill")
    virtual void TryActivateSkill(FName SkillID);

    // 스킬 슬롯에 배치
    UFUNCTION(BlueprintCallable, Category = "Skill")
    void AssignSkillToSlot(FName SkillID, int32 SlotIndex);

    // 슬롯에서 SkillID 조회
    UFUNCTION(BlueprintCallable, Category = "Skill")
    FName GetSkillAtSlot(int32 SlotIndex) const;

    // 슬롯의 남은 쿨다운 시간을 반환
    UFUNCTION(BlueprintCallable)
    float GetCooldownRemaining(int32 SlotIndex) const;

    // DT에서 스킬 데이터 조회
    FPTSkillRow* GetSkillData(FName SkillID) const;

    // 스킬 몽타주 및 이펙트/사운드를 전체 클라이언트에 전파
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlaySkillMontage(UAnimMontage* Montage, UNiagaraSystem* Effect, USoundBase* Sound);

protected:
    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void BeginPlay() override;

    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 쿨다운 종료 처리
    virtual void OnCooldownEnd(int32 SlotIndex);

public:
    // ── 멤버 변수 ────────────────────────────────────────────────────────────

    // DT 참조
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<UDataTable> SkillDataTable;

    // 스킬 슬롯 (Q, W, E, R)
    UPROPERTY(EditAnywhere, Category = "Skill")
    TArray<FName> SkillSlots;

    // 현재 발동 중인 스킬 ID
    UPROPERTY()
    FName CurrentSkillID = NAME_None;

protected:
    // ── 멤버 변수 (protected) ────────────────────────────────────────────────

    // 쿨다운 타이머 (슬롯 당 하나)
    TArray<FTimerHandle> CooldownTimers;

    // 쿨다운 중인 슬롯 플래그
    TArray<bool> bIsCooldown;
};
