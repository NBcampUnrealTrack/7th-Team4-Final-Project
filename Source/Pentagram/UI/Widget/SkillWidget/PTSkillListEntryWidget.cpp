#include "PTSkillListEntryWidget.h"
#include "PTSkillDragDropOperation.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PTSkillEntryObject.h"

void UPTSkillListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

    UPTSkillEntryObject* Entry = Cast<UPTSkillEntryObject>(ListItemObject);
    if (!Entry) return;

    SkillID = Entry->SkillID;
    CachedRow = Entry->SkillRow;

    if (Img_Icon)
    {
        if (!CachedRow.SkillIcon.IsNull())
        {
            // 크기는 WBP(SizeBox/Slot Alignment)에서 전적으로 제어. 코드에서 크기 개입 안 함.
            Img_Icon->SetBrushFromSoftTexture(CachedRow.SkillIcon, false);
            Img_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            Img_Icon->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (Txt_Name) Txt_Name->SetText(CachedRow.SkillName);
    if (Txt_Type) Txt_Type->SetText(CachedRow.SkillTypeName);
    if (Txt_Rank) Txt_Rank->SetText(FText::GetEmpty()); // 랭크 미구현
    if (Txt_Cost) Txt_Cost->SetText(FText::AsNumber(FMath::RoundToInt(CachedRow.MPCost)));
}

FReply UPTSkillListEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (SkillID == NAME_None) return FReply::Unhandled();
    if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) return FReply::Unhandled();

    // 드래그 감지
    return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
}

FReply UPTSkillListEntryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (SkillID != NAME_None && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // 클릭 = 상세정보 표시
        OnEntryClicked.Broadcast(SkillID, CachedRow);
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UPTSkillListEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (SkillID == NAME_None) return;

    UPTSkillDragDropOperation* Op = NewObject<UPTSkillDragDropOperation>(this);
    if (!Op) return;

    Op->SkillID = SkillID;
    Op->SkillIcon = CachedRow.SkillIcon;
    Op->Pivot = EDragPivot::MouseDown;
    Op->DefaultDragVisual = CreateDragVisual();

    OutOperation = Op;
}

UWidget* UPTSkillListEntryWidget::CreateDragVisual() const
{
    // WBP 우선 (원형 마스크는 여기서 처리)
    if (DragVisualClass)
    {
        return CreateWidget<UUserWidget>(GetOwningPlayer(), DragVisualClass);
    }

    // 폴백 아이콘
    if (CachedRow.SkillIcon.IsNull()) return nullptr;

    UImage* Ghost = NewObject<UImage>(const_cast<UPTSkillListEntryWidget*>(this));
    if (!Ghost) return nullptr;

    Ghost->SetBrushFromSoftTexture(CachedRow.SkillIcon, false);
    Ghost->Brush.ImageSize = FVector2D(64.f, 64.f);
    return Ghost;
}
