#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Pentagram/Character/Skill/PTSkillRow.h"
#include "PTSkillListEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UUserWidget;
class UDragDropOperation;

// 리스트 항목 클릭
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnSkillEntryClicked, FName, SkillID, FPTSkillRow, SkillRow);

// 스킬 리스트 한 줄 위젯
UCLASS()
class PENTAGRAM_API UPTSkillListEntryWidget : public UCommonUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    // ── 델리게이트 ──
    UPROPERTY(BlueprintAssignable, Category = "PT|UI|Skill")
    FPTOnSkillEntryClicked OnEntryClicked;

protected:
    // ── 오버라이드 ──
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
    // 드래그 비주얼 생성
    UWidget* CreateDragVisual() const;

protected:
    // ── 위젯 바인딩 ──
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_Icon;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Name;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Type;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Rank;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Cost;

    // 드래그 비주얼 (원형 WBP 권장, 없으면 아이콘 폴백)
    UPROPERTY(EditAnywhere, Category = "PT|UI|Skill")
    TSubclassOf<UUserWidget> DragVisualClass;

private:
    // ── 멤버 변수 ──
    FName SkillID = NAME_None;
    FPTSkillRow CachedRow;
};
