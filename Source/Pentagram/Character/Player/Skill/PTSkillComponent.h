#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/Data/PTDelegates.h"
#include "PTSkillRow.h"
#include "PTSkillComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTSkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPTSkillComponent();

    // 스킬 발동 시도
    UFUNCTION(BlueprintCallable, Category = "Skill")
    void TryActivateSkill(FName SkillID);

    // 스킬슬롯에 배치
    UFUNCTION(BlueprintCallable, Category = "Skill")
    void AssignSkillToSlot(FName SkillID, int32 SlotIndex);

    // 슬롯에서 SkillID 조회
    UFUNCTION(BlueprintCallable, Category = "Skill")
    FName GetSkillAtSlot(int32 SlotIndex) const;

    //DT 참조
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<UDataTable> SkillDataTable;

    // 스킬 슬롯 (Q, W, E, R)
    UPROPERTY(EditAnywhere, Category = "Skill")
    TArray<FName> SkillSlots;

    UPROPERTY()
    FName CurrentSkillID = NAME_None;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnSkillCooldownEnd, int32, SlotIndex);

    //델리게이트의 인스턴스
    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownEnd OnSkillCooldownEnd;

    UPROPERTY(BlueprintAssignable)
    FPTOnSkillCooldownStart OnSkillCooldownStart;

    UFUNCTION(Client, Reliable)
    void Client_NotifyCooldownStarted(int32 SlotIndex, float Duration);

    // 슬롯의 남은 쿨다운 시간을 반환
    UFUNCTION(BlueprintCallable)
    float GetCooldownRemaining(int32 SlotIndex) const;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlaySkillMontage(UAnimMontage* Montage);

    //DT에서 스킬 데이터 조회
    FPTSkillRow* GetSkillData(FName SkillID) const;

protected:
    virtual void BeginPlay() override;

    //쿨타운 타이머 (슬롯 당 하나)
    TArray<FTimerHandle> CooldownTimers;

    //쿨다운 스킬 체크
    TArray<bool> bIsCooldown;

    //쿨다운 중인 슬롯 체크
    void OnCooldownEnd(int32 SlotIndex);

};
