#include "PTSkillComponent.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTBaseCharacter.h"

UPTSkillComponent::UPTSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

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
    if (!GetOwner()->HasAuthority()) return;

    // DT에서 스킬 데이터 조회
    const FPTSkillRow* SkillData = GetSkillData(SkillID);
    if (!SkillData) return;

    // 슬롯 인덱스 찾기
    int32 SlotIndex = SkillSlots.IndexOfByKey(SkillID);
    if (SlotIndex == INDEX_NONE) return;

    // 쿨다운 체크
    if (bIsCooldown[SlotIndex]) return;

    // MP 체크 및 차감
    APTBaseCharacter* Owner = Cast<APTBaseCharacter>(GetOwner());
    if (!Owner) return;

    if (Owner->CurrentMP < SkillData->MPCost) return;
    Owner->CurrentMP -= SkillData->MPCost;

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

    // 몽타주 재생
    if (UAnimMontage* Montage = SkillData->SkillMontage.LoadSynchronous())
    {
        Owner->PlayAnimMontage(Montage);
    }
}

void UPTSkillComponent::OnCooldownEnd(int32 SlotIndex)
{
    bIsCooldown[SlotIndex] = false;
    // UI 쿨다운 종료 델리게이트 발행 (나중에 연동)
}



