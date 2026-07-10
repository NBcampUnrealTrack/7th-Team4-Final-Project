#include "PTSkillWindowWidget.h"
#include "PTSkillEntryObject.h"
#include "PTSkillListEntryWidget.h"
#include "Pentagram/UI/Widget/Widget/SkillSlot/PTSkillSlotWidget.h"
#include "Components/ListView.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Character/Player/PTPlayerController.h"
#include "Character/Skill/PTPlayerSkillComponent.h"

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

    if (UPTPlayerSkillComponent* SC = GetSkillComponent()) SC->OnSkillLearned.AddUniqueDynamic(this, &UPTSkillWindowWidget::HandleSkillLearned);
}

void UPTSkillWindowWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    RefreshSkillList();
    ClearSkillDetail();
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

bool UPTSkillWindowWidget::IsScreenPositionOverContent_Implementation(const FVector2D& ScreenPosition) const
{
    if (!Img_Background)
    {
        return false;
    }

    return Img_Background->GetCachedGeometry().IsUnderLocation(ScreenPosition);
}

void UPTSkillWindowWidget::RefreshSkillList()
{
    if (!SkillListView) return;

    SkillListView->ClearListItems();
    SkillEntries.Reset();

    UPTPlayerSkillComponent* SkillComp = GetSkillComponent();
    if (!SkillComp || !SkillDataTable) return;

    for (const FName& SkillID : SkillComp->LearnedSkills)
    {
        const FPTSkillRow* Row = SkillDataTable->FindRow<FPTSkillRow>(SkillID, TEXT("RefreshSkillList"));
        if (!Row) continue;

        UPTSkillEntryObject* Entry = NewObject<UPTSkillEntryObject>(this);
        Entry->SkillID  = SkillID;
        Entry->SkillRow = *Row;
        SkillEntries.Add(Entry);
        SkillListView->AddItem(Entry);
    }

   /* UE_LOG(LogTemp, Warning, TEXT("[SkillList] RefreshSkillList 호출, SkillListView=%d, SkillDataTable=%d"),
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
    */
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

    if (APTPlayerController* PC = Cast<APTPlayerController>(GetOwningPlayer()))
    {
        PC->Server_RequestAssignSkillToSlot(SkillID, SlotIndex);
    }
}

void UPTSkillWindowWidget::HandleSkillLearned(FName SkillID)
{
    RefreshSkillList();
}

void UPTSkillWindowWidget::ShowSkillDetail(const FPTSkillRow& Row)
{
    if (Img_DetailIcon)
    {
        if (!Row.SkillIcon.IsNull())
        {
            // 크기는 WBP(SizeBox/Slot Alignment)에서 전적으로 제어. 코드에서 크기 개입 안 함.
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

UPTPlayerSkillComponent* UPTSkillWindowWidget::GetSkillComponent() const
{
    if (APawn* P = GetOwningPlayerPawn())
        return P->FindComponentByClass<UPTPlayerSkillComponent>();
    return nullptr;
}
