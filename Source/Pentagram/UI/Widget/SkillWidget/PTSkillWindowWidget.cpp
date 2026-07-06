#include "PTSkillWindowWidget.h"
#include "PTSkillEntryObject.h"
#include "PTSkillListEntryWidget.h"
#include "Pentagram/UI/Widget/Widget/SkillSlot/PTSkillSlotWidget.h"
#include "Components/ListView.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Character/Player/PTPlayerController.h"

void UPTSkillWindowWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (SkillListView)
    {
        SkillListView->OnEntryWidgetGenerated().AddUObject(this, &UPTSkillWindowWidget::HandleEntryWidgetGenerated);
    }

    if (EquippedSlots)
    {
        EquippedSlots->OnSkillAssignRequested.AddUniqueDynamic(this, &UPTSkillWindowWidget::HandleSkillAssignRequested);
    }
}

void UPTSkillWindowWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    RefreshSkillList();
    ClearSkillDetail();

    // 스킬창은 인벤토리와 달리 열려있는 동안에도 마우스 이동/공격이 계속 가능해야 하므로
    // 게임 입력을 막지 않는다 (SetGameplayInputBlockedByUI 호출 없음)
}

void UPTSkillWindowWidget::NativeOnDeactivated()
{
    if (APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PC->RestoreGameplayInput();
    }

    Super::NativeOnDeactivated();
}

bool UPTSkillWindowWidget::NativeOnHandleBackAction()
{
    DeactivateWidget();
    return true;
}

void UPTSkillWindowWidget::RefreshSkillList()
{
    UE_LOG(LogTemp, Warning, TEXT("[SkillList] RefreshSkillList 호출, SkillListView=%d, SkillDataTable=%d"),
        SkillListView != nullptr, SkillDataTable != nullptr);

    if (!SkillListView || !SkillDataTable) return;

    SkillListView->ClearListItems();
    SkillEntries.Reset();

    TArray<FName> RowNames = SkillDataTable->GetRowNames();
    UE_LOG(LogTemp, Warning, TEXT("[SkillList] RowNames 개수=%d"), RowNames.Num());

    for (const FName& RowName : RowNames)
    {
        const FPTSkillRow* Row = SkillDataTable->FindRow<FPTSkillRow>(RowName, TEXT("RefreshSkillList"));
        if (!Row)
        {
            UE_LOG(LogTemp, Warning, TEXT("[SkillList] Row 조회 실패: %s"), *RowName.ToString());
            continue;
        }

        UPTSkillEntryObject* Entry = NewObject<UPTSkillEntryObject>(this);
        Entry->SkillID = RowName;
        Entry->SkillRow = *Row;
        SkillEntries.Add(Entry);

        UE_LOG(LogTemp, Warning, TEXT("[SkillList] 항목 추가: %s"), *RowName.ToString());
    }

    UE_LOG(LogTemp, Warning, TEXT("[SkillList] SkillEntries 총 개수=%d"), SkillEntries.Num());

    for (UPTSkillEntryObject* Entry : SkillEntries)
    {
        SkillListView->AddItem(Entry);
    }

    UE_LOG(LogTemp, Warning, TEXT("[SkillList] ListView에 AddItem 완료"));
}

void UPTSkillWindowWidget::HandleEntryWidgetGenerated(UUserWidget& EntryWidget)
{
    if (UPTSkillListEntryWidget* SkillEntry = Cast<UPTSkillListEntryWidget>(&EntryWidget))
    {
        SkillEntry->OnEntryClicked.AddUniqueDynamic(this, &UPTSkillWindowWidget::HandleSkillEntryClicked);
    }
}

void UPTSkillWindowWidget::HandleSkillEntryClicked(FName SkillID, FPTSkillRow SkillRow)
{
    SelectedSkillID = SkillID;
    ShowSkillDetail(SkillRow);
}

void UPTSkillWindowWidget::HandleSkillAssignRequested(int32 SlotIndex, FName SkillID)
{
    if (SkillID == NAME_None) return;

    // 서버에 배정 요청 → 성공 시 Client RPC로 모든 슬롯바 인스턴스(HUD 포함)가 자동 동기화됨
    if (APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PC->Server_RequestAssignSkillToSlot(SkillID, SlotIndex);
    }
}

void UPTSkillWindowWidget::ShowSkillDetail(const FPTSkillRow& Row)
{
    if (Img_DetailIcon)
    {
        if (!Row.SkillIcon.IsNull())
        {
            Img_DetailIcon->SetBrushFromSoftTexture(Row.SkillIcon, false);
            Img_DetailIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            Img_DetailIcon->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (Txt_DetailName)     Txt_DetailName->SetText(Row.SkillName);
    if (Txt_DetailType)     Txt_DetailType->SetText(Row.SkillTypeName);
    if (Txt_DetailRank)     Txt_DetailRank->SetText(FText::GetEmpty()); // 랭크 미구현
    if (Txt_DetailAttack)   Txt_DetailAttack->SetText(FText::AsNumber(Row.DamageMultiplier));
    if (Txt_DetailManaCost) Txt_DetailManaCost->SetText(FText::AsNumber(FMath::RoundToInt(Row.MPCost)));
    if (Txt_DetailCooldown) Txt_DetailCooldown->SetText(FText::AsNumber(Row.Cooldown));
}

void UPTSkillWindowWidget::ClearSkillDetail()
{
    SelectedSkillID = NAME_None;

    if (Img_DetailIcon)     Img_DetailIcon->SetVisibility(ESlateVisibility::Hidden);
    if (Txt_DetailName)     Txt_DetailName->SetText(FText::GetEmpty());
    if (Txt_DetailType)     Txt_DetailType->SetText(FText::GetEmpty());
    if (Txt_DetailRank)     Txt_DetailRank->SetText(FText::GetEmpty());
    if (Txt_DetailAttack)   Txt_DetailAttack->SetText(FText::GetEmpty());
    if (Txt_DetailManaCost) Txt_DetailManaCost->SetText(FText::GetEmpty());
    if (Txt_DetailCooldown) Txt_DetailCooldown->SetText(FText::GetEmpty());
}
