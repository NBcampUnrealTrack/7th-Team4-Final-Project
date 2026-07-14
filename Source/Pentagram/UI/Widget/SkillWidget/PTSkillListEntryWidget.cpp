#include "PTSkillListEntryWidget.h"
#include "PTSkillDragDropOperation.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PTSkillEntryObject.h"
#include "PTSkillWindowWidget.h"

void UPTSkillListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

    UPTSkillEntryObject* Entry = Cast<UPTSkillEntryObject>(ListItemObject);
    if (!Entry) return;
    UPTSkillWindowWidget* Window = Cast<UPTSkillWindowWidget>(ListItemObject);
    SkillID = Entry->SkillID;
    CachedRow = Entry->SkillRow;

    if (Img_Icon)
    {
        if (!CachedRow.SkillIcon.IsNull())
        {
            Img_Icon->SetBrushFromSoftTexture(CachedRow.SkillIcon, false);
            Img_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            Img_Icon->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (Txt_Name) Txt_Name->SetText(CachedRow.SkillName);
    if (Txt_Type) Txt_Type->SetText(Window->GetTargetingModeDisplayText(CachedRow.TargetingMode));
    if (Txt_Rank) Txt_Rank->SetText(FText::AsNumber(FMath::RoundToInt(CachedRow.MPCost)));
    if (Txt_Cost) Txt_Cost->SetText(FText::AsNumber(FMath::RoundToInt(CachedRow.MPCost)));
}

FReply UPTSkillListEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (SkillID == NAME_None) return FReply::Unhandled();
    if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) return FReply::Unhandled();

    return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
}

FReply UPTSkillListEntryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (SkillID != NAME_None && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
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
    if (DragVisualClass)
    {
        UUserWidget* DragVisual = CreateWidget<UUserWidget>(GetOwningPlayer(), DragVisualClass);
        if (DragVisual)
        {
            if (UImage* IconImg = Cast<UImage>(DragVisual->GetWidgetFromName(TEXT("Img_Icon_UseCircularMaskMaterialHere"))))
            {
                if (UMaterialInstanceDynamic* MID = IconImg->GetDynamicMaterial())
                {
                    UTexture2D* IconTexture = CachedRow.SkillIcon.LoadSynchronous();
                    if (IconTexture)
                    {
                        MID->SetTextureParameterValue(TEXT("Icon"), IconTexture);
                    }
                }
            }
        }
        return DragVisual;
    }

    if (CachedRow.SkillIcon.IsNull()) return nullptr;

    UImage* Ghost = NewObject<UImage>(const_cast<UPTSkillListEntryWidget*>(this));
    if (!Ghost) return nullptr;

    Ghost->SetBrushFromSoftTexture(CachedRow.SkillIcon, false);
    Ghost->Brush.ImageSize = FVector2D(0.f, 0.f);
    return Ghost;
}
