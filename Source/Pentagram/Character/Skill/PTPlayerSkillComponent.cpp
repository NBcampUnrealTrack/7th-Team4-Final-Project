#include "PTPlayerSkillComponent.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTPlayerCharacter.h"

void UPTPlayerSkillComponent::TryActivateSkill(FName SkillID)
{
    UE_LOG(LogTemp, Warning, TEXT("TryActivateSkill 호출 - SkillID: %s"), *SkillID.ToString());

    CurrentSkillID = SkillID;

    if (!GetOwner()->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 권한 없음 - 서버가 아님"));
        return;
    }

    // DT에서 스킬 데이터 조회
    const FPTSkillRow* SkillData = GetSkillData(SkillID);
    if (!SkillData)
    {
        UE_LOG(LogTemp, Warning, TEXT("DT에서 스킬 데이터 없음 - SkillID: %s"), *SkillID.ToString());
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Skill 데이터 조회 성공 - MP소모: %.1f, 쿨다운: %.1f"), SkillData->MPCost, SkillData->Cooldown);

    // 슬롯 인덱스 찾기
    int32 SlotIndex = SkillSlots.IndexOfByKey(SkillID);
    if (SlotIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 슬롯에 등록되지 않은 스킬"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Skill 슬롯 인덱스: %d"), SlotIndex);

    // 쿨다운 체크
    if (bIsCooldown[SlotIndex])
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 쿨다운 중 - 남은 시간: %.1f초"), GetCooldownRemaining(SlotIndex));
        return;
    }

    // MP 체크 및 차감
    APTBaseCharacter* Owner = Cast<APTBaseCharacter>(GetOwner());
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill Owner 실패"));
        return;
    }

    if (Owner->CurrentMP < SkillData->MPCost)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill MP 부족 - 현재: %.1f, 필요: %.1f"), Owner->CurrentMP, SkillData->MPCost);
        return;
    }
    Owner->CurrentMP -= SkillData->MPCost;
    UE_LOG(LogTemp, Warning, TEXT("Skill 스킬 발동 성공 - 남은 MP: %.1f"), Owner->CurrentMP);

    // PlayerState MP 반영
    APTBasePlayerState* PS = Owner->GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentMP = Owner->CurrentMP;
    }

    // 쿨다운 시작
    bIsCooldown[SlotIndex] = true;
    GetWorld()->GetTimerManager().SetTimer(
        CooldownTimers[SlotIndex],
        [this, SlotIndex]() { OnCooldownEnd(SlotIndex); },
        SkillData->Cooldown,
        false
    );

    // ★ 발동 성공한 이 시점에 소유 클라로 "쿨다운 시작" 통지
    Client_NotifyCooldownStarted(SlotIndex, SkillData->Cooldown);

    if (UAnimMontage* Montage = SkillData->SkillMontage.LoadSynchronous())
    {
        APTPlayerCharacter* PlayerCharacter = Cast<APTPlayerCharacter>(Owner);
        if (PlayerCharacter)
        {
            PlayerCharacter->bIsAttacking = false;
            PlayerCharacter->bCanCombo = false;
            PlayerCharacter->ComboIndex = 0;
            Owner->StopAnimMontage();
        }

        // 이펙트/사운드 에셋 로드 (서버에서 한 번만 로드 후 Multicast로 전달)
        UNiagaraSystem* Effect = SkillData->SkillEffect.LoadSynchronous();
        USoundBase* Sound = SkillData->SkillSound.LoadSynchronous();
        Multicast_PlaySkillMontage(Montage, Effect, Sound);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 몽타주 없음"));
    }
}

void UPTPlayerSkillComponent::OnCooldownEnd(int32 SlotIndex)
{
    Super::OnCooldownEnd(SlotIndex);

    // UI 쿨다운 종료 델리게이트 발행
    OnSkillCooldownEnd.Broadcast(SlotIndex);
}

void UPTPlayerSkillComponent::Client_NotifyCooldownStarted_Implementation(int32 SlotIndex, float Duration)
{
    OnSkillCooldownStart.Broadcast(SlotIndex, Duration);
}
