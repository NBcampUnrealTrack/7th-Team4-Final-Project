#include "PTSkillSlotWidget.h"
#include "PTSkillSlotEntryWidget.h"

void UPTSkillSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 슬롯 배열화
    Entries = { Slot_Q, Slot_W, Slot_E, Slot_R };
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
