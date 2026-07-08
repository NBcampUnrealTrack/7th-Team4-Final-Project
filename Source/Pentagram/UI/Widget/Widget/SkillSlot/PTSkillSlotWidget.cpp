#include "PTSkillSlotWidget.h"
#include "PTSkillSlotEntryWidget.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTPlayerSkillComponent.h"

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

void UPTSkillSlotWidget::InitWithSkillComponent(UPTPlayerSkillComponent* InSkillComp)
{
    if (!InSkillComp) return;

    // 중복 해제
    if (SkillComp.IsValid())
    {
        SkillComp->OnSkillCooldownStart.RemoveDynamic(this, &UPTSkillSlotWidget::HandleCooldownStart);
        SkillComp->OnSkillCooldownEnd.RemoveDynamic(this, &UPTSkillSlotWidget::HandleCooldownEnd);
        SkillComp->OnSkillSlotAssigned.RemoveDynamic(this, &UPTSkillSlotWidget::HandleSlotAssigned);
    }

    SkillComp = InSkillComp;
    SkillComp->OnSkillCooldownStart.AddDynamic(this, &UPTSkillSlotWidget::HandleCooldownStart);
    SkillComp->OnSkillCooldownEnd.AddDynamic(this, &UPTSkillSlotWidget::HandleCooldownEnd);
    SkillComp->OnSkillSlotAssigned.AddDynamic(this, &UPTSkillSlotWidget::HandleSlotAssigned);

    // 이미 배정된 스킬 아이콘 즉시 반영 (스킬창을 나중에 열어도 HUD와 동일하게 보이도록)
    for (int32 i = 0; i < Entries.Num(); ++i)
    {
        const FName AssignedID = SkillComp->GetSkillAtSlot(i);
        if (AssignedID != NAME_None)
        {
            if (const FPTSkillRow* Row = SkillComp->GetSkillData(AssignedID))
            {
                SetSlotIcon(i, Row->SkillIcon.LoadSynchronous());
            }
        }
    }
}

void UPTSkillSlotWidget::RefreshUsability()
{
    if (!SkillComp.IsValid()) return;
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwningPlayerPawn());
    if (!PC) return;

    for (int32 i = 0; i < Entries.Num(); ++i)
    {
        const FName ID = SkillComp->GetSkillAtSlot(i);
        bool bUsable = false;
        if (ID != NAME_None)
            if (const FPTSkillRow* Row = SkillComp->GetSkillData(ID))
                bUsable = Row->IsWeaponAllowed(PC->CurrentWeaponType);

        SetSlotUsable(i, bUsable);
    }
}

void UPTSkillSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 슬롯 배열화
    Entries = { Slot_Q, Slot_W, Slot_E, Slot_R };

    for (int32 i = 0; i < Entries.Num(); ++i)
    {
        if (Entries[i]) Entries[i]->SetSlotIndex(i);
    }

    // 드롭 바인딩
    if (Slot_Q) Slot_Q->OnSkillDropped.AddUniqueDynamic(this, &UPTSkillSlotWidget::HandleSlotQDropped);
    if (Slot_W) Slot_W->OnSkillDropped.AddUniqueDynamic(this, &UPTSkillSlotWidget::HandleSlotWDropped);
    if (Slot_E) Slot_E->OnSkillDropped.AddUniqueDynamic(this, &UPTSkillSlotWidget::HandleSlotEDropped);
    if (Slot_R) Slot_R->OnSkillDropped.AddUniqueDynamic(this, &UPTSkillSlotWidget::HandleSlotRDropped);

    if (APawn* P = GetOwningPlayerPawn())
    {
        if (UPTPlayerSkillComponent* SC = P->FindComponentByClass<UPTPlayerSkillComponent>())
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
        SkillComp->OnSkillSlotAssigned.RemoveDynamic(this, &UPTSkillSlotWidget::HandleSlotAssigned);
    }
    Super::NativeDestruct();
}

void UPTSkillSlotWidget::HandleCooldownStart(int32 SlotIndex, float Duration)
{
    StartSlotCooldown(SlotIndex, Duration);
}

void UPTSkillSlotWidget::HandleCooldownEnd(int32 SlotIndex)
{
    // 즉시 종료(선택)
    if (UPTSkillSlotEntryWidget* E = GetEntry(SlotIndex)) E->SetCooldownRemaining(0.f, 0.f);
}

void UPTSkillSlotWidget::HandleSlotAssigned(int32 SlotIndex, FName SkillID)
{
    if (!SkillComp.IsValid()) return;

    if (const FPTSkillRow* Row = SkillComp->GetSkillData(SkillID))
    {
        SetSlotIcon(SlotIndex, Row->SkillIcon.LoadSynchronous());
    }
}

void UPTSkillSlotWidget::HandleSlotQDropped(FName SkillID) { OnSkillAssignRequested.Broadcast(0, SkillID); }
void UPTSkillSlotWidget::HandleSlotWDropped(FName SkillID) { OnSkillAssignRequested.Broadcast(1, SkillID); }
void UPTSkillSlotWidget::HandleSlotEDropped(FName SkillID) { OnSkillAssignRequested.Broadcast(2, SkillID); }
void UPTSkillSlotWidget::HandleSlotRDropped(FName SkillID) { OnSkillAssignRequested.Broadcast(3, SkillID); }

UPTSkillSlotEntryWidget* UPTSkillSlotWidget::GetEntry(int32 SlotIndex) const
{
    // 슬롯 찾기
    return Entries.IsValidIndex(SlotIndex) ? Entries[SlotIndex] : nullptr;
}
