#include "PTSkillComponent.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTPlayerCharacter.h"

UPTSkillComponent::UPTSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    //스킬 슬롯 초기화
    SkillSlots.Init(NAME_None, 4);
    CooldownTimers.SetNum(4);
    bIsCooldown.Init(false, 4);
}

void UPTSkillComponent::BeginPlay()
{
    Super::BeginPlay();
}

FPTSkillRow* UPTSkillComponent::GetSkillData(FName SkillID) const
{
    if (!SkillDataTable) return nullptr;
    return SkillDataTable->FindRow<FPTSkillRow>(SkillID, TEXT("GetSkillData"));
}

void UPTSkillComponent::AssignSkillToSlot(FName SkillID, int32 SlotIndex)
{
    if (!SkillSlots.IsValidIndex(SlotIndex)) return;
    SkillSlots[SlotIndex] = SkillID;
}

FName UPTSkillComponent::GetSkillAtSlot(int32 SlotIndex) const
{
    if (!SkillSlots.IsValidIndex(SlotIndex)) return NAME_None;
    return SkillSlots[SlotIndex];
}

void UPTSkillComponent::TryActivateSkill(FName SkillID)
{
    UE_LOG(LogTemp,Warning, TEXT("TryActivateSkill 호출 - SkillID: %s"), *SkillID.ToString());

    CurrentSkillID = SkillID;

    if (!GetOwner()->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill권한 없음 - 서버가 아님"));
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
        Multicast_PlaySkillMontage(Montage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 몽타주 없음"));
    }
}

void UPTSkillComponent::OnCooldownEnd(int32 SlotIndex)
{
    bIsCooldown[SlotIndex] = false;
    UE_LOG(LogTemp, Warning, TEXT("Skill 쿨다운 종료 - 슬롯: %d"), SlotIndex);
    OnSkillCooldownEnd.Broadcast(SlotIndex);
    // UI 쿨다운 종료 델리게이트 발행 (나중에 연동)
}

float UPTSkillComponent::GetCooldownRemaining(int32 SlotIndex) const
{
    if (!bIsCooldown[SlotIndex]) return 0.f;

    return GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimers[SlotIndex]);
    //쿨다운이 끝났을 때 발행하는 델리게이트
}

void UPTSkillComponent::Multicast_PlaySkillMontage_Implementation(UAnimMontage* Montage)
{
    if (!Montage) return;

    APTBaseCharacter* Owner = Cast<APTBaseCharacter>(GetOwner());
    if (!Owner) return;

    Owner->PlayAnimMontage(Montage);
}


