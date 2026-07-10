#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTSkillRow.h"
#include "PTSkillComponent.generated.h"

USTRUCT(BlueprintType)
struct FPTSkillActivationRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SkillRowName = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UDataTable> SkillDataTable = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UAnimMontage> OverrideMontage;

    // 조준 데이터 (Server_UseSkill이 채워서 넘김)
    UPROPERTY(BlueprintReadWrite) FVector_NetQuantize       TargetLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite) FVector_NetQuantizeNormal AimDirection   = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite) TObjectPtr<AActor>        TargetActor    = nullptr;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTSkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPTSkillComponent();

    // 스킬 발동 시도
    UFUNCTION(BlueprintCallable, Category = "PT|Skill")
    virtual void TryActivateSkill(const FPTSkillActivationRequest& Request);

    virtual bool TryActivateSkillChecked(const FPTSkillActivationRequest& Request);

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

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

    UFUNCTION(BlueprintPure, Category = "PT|Skill")
    FName GetCurrentSkillID() const { return CurrentSkillID; }

    // 스킬 몽타주 및 이펙트/사운드를 전체 클라이언트에 전파
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlaySkillMontage(UAnimMontage* Montage, UNiagaraSystem* Effect, USoundBase* Sound);

    // 보스 스킬용 — Offset을 직접 전달받아 SkillDataTable 조회 불필요
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlaySkillMontageWithOffset(UAnimMontage* Montage, UNiagaraSystem* Effect, USoundBase* Sound, FVector SkillOffset, FName SkillID);

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayHitSound(USoundBase* Sound, FVector Location);

    // 월드 지점에 몽타주+이펙트 (지점형 AoE 스킬용)
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlaySkillMontageAtLocation(UAnimMontage* Montage, UNiagaraSystem* Effect, USoundBase* Sound, FVector WorldLocation, FName SkillID);

protected:

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 쿨다운 종료 처리
    virtual void OnCooldownEnd(int32 SlotIndex);

    const FPTSkillRow* FindSkillRowFromRequest(const FPTSkillActivationRequest& Request) const;

public:

    // DT 참조
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<UDataTable> SkillDataTable;

    // 스킬 슬롯 (Q, W, E, R)
    UPROPERTY(EditAnywhere, Replicated, Category = "Skill")
    TArray<FName> SkillSlots;

    UPROPERTY(Replicated, BlueprintReadWrite)
    FVector_NetQuantize TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    FVector_NetQuantizeNormal AimDirection = FVector::ZeroVector;

    UPROPERTY(Replicated, BlueprintReadWrite)
    TObjectPtr<AActor> TargetActor = nullptr;

    // 쿨다운 중인 슬롯 플래그
    TArray<bool> bIsCooldown;
protected:

     // 현재 발동 중인 스킬 ID
    UPROPERTY()
    FName CurrentSkillID = NAME_None;

    // 쿨다운 타이머 (슬롯 당 하나)
    TArray<FTimerHandle> CooldownTimers;

};
