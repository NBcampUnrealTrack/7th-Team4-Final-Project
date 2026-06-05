#include "PTSkillSlotWidget.h"
#include "PTSkillSlotEntryWidget.h"
#include "Character/Player/Skill/PTSkillComponent.h"

void UPTSkillSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 슬롯 배열화
    Entries = { Slot_Q, Slot_W, Slot_E, Slot_R };

    if (APawn* P = GetOwningPlayerPawn())
    {
        if (UPTSkillComponent* SC = P->FindComponentByClass<UPTSkillComponent>())
        {
            InitWithSkillComponent(SC);
        }
    }
}

void UPTSkillSlotWidget::NativeDestruct()
{
    if (SkillComp.IsValid())
    {
        SkillComp->OnSkillCooldownStart.RemoveDynamic(this, &UPTSkillSlotWidget::HandleCooldownStart);
        SkillComp->OnSkillCooldownEnd.RemoveDynamic(this, &UPTSkillSlotWidget::HandleCooldownEnd);
    }
    Super::NativeDestruct();
}

void UPTSkillSlotWidget::InitWithSkillComponent(UPTSkillComponent* InSkillComp) // ★
{
    if (!InSkillComp) return;

    // 기존 연결 해제(중복 바인드 방지)
    if (SkillComp.IsValid())
    {
        SkillComp->OnSkillCooldownStart.RemoveDynamic(this, &UPTSkillSlotWidget::HandleCooldownStart);
        SkillComp->OnSkillCooldownEnd.RemoveDynamic(this, &UPTSkillSlotWidget::HandleCooldownEnd);
    }

    SkillComp = InSkillComp;
    SkillComp->OnSkillCooldownStart.AddDynamic(this, &UPTSkillSlotWidget::HandleCooldownStart);
    SkillComp->OnSkillCooldownEnd.AddDynamic(this, &UPTSkillSlotWidget::HandleCooldownEnd);
}

void UPTSkillSlotWidget::HandleCooldownStart(int32 SlotIndex, float Duration) // ★
{
    StartSlotCooldown(SlotIndex, Duration);
}

void UPTSkillSlotWidget::HandleCooldownEnd(int32 SlotIndex) // ★
{
    // 위젯이 NativeTick으로 0까지 알아서 내려가므로 보통 비워둬도 됨.
    // 즉시 끄고 싶으면 아래 한 줄 사용:
    if (UPTSkillSlotEntryWidget* E = GetEntry(SlotIndex)) E->SetCooldownRemaining(0.f, 0.f);
}

UPTSkillSlotEntryWidget* UPTSkillSlotWidget::GetEntry(int32 SlotIndex) const
{
    // 슬롯 찾기
    return Entries.IsValidIndex(SlotIndex) ? Entries[SlotIndex] : nullptr;
}

void UPTSkillSlotWidget::SetSlotIcon(int32 SlotIndex, UTexture2D* Icon)
{
    // 아이콘 변경
    if (UPTSkillSlotEntryWidget* E = GetEntry(SlotIndex)) E->SetIcon(Icon);
}

void UPTSkillSlotWidget::StartSlotCooldown(int32 SlotIndex, float Duration)
{
    // 쿨다운 시작
    if (UPTSkillSlotEntryWidget* E = GetEntry(SlotIndex)) E->StartCooldown(Duration);
}

void UPTSkillSlotWidget::SetSlotUsable(int32 SlotIndex, bool bUsable)
{
    // 사용가능 설정
    if (UPTSkillSlotEntryWidget* E = GetEntry(SlotIndex)) E->SetUsable(bUsable);
}
