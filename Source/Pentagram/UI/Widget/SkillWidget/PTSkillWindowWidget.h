#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Pentagram/Character/Skill/PTSkillRow.h"
#include "PTSkillWindowWidget.generated.h"

class UListView;
class UImage;
class UTextBlock;
class UPTSkillSlotWidget;
class UPTSkillEntryObject;
class UUserWidget;

UCLASS()
class PENTAGRAM_API UPTSkillWindowWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    // 스킬 목록 다시 그리기
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void RefreshSkillList();

protected:
    // ── 오버라이드 ──
    virtual void NativeOnInitialized() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

    // 항목 위젯 생성 시점 (클릭 델리게이트 바인딩용)
    void HandleEntryWidgetGenerated(UUserWidget& EntryWidget);

    UFUNCTION()
    void HandleSkillEntryClicked(FName SkillID, FPTSkillRow SkillRow);

    UFUNCTION()
    void HandleSkillAssignRequested(int32 SlotIndex, FName SkillID);

    void ShowSkillDetail(const FPTSkillRow& Row);
    void ClearSkillDetail();

protected:
    // ── 위젯 바인딩 ──
    // 스킬 리스트 (최대 6줄 노출, 나머지 스크롤 — 높이/엔트리크기는 WBP에서 설정)
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UListView> SkillListView;

    // 하단 QWER 장착 슬롯 (기존 HUD 위젯 재사용)
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPTSkillSlotWidget> EquippedSlots;

    // 상세정보 패널
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_DetailIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_DetailName;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_DetailType;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_DetailRank;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_DetailAttack;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_DetailManaCost;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_DetailCooldown;

    // ── 설정 ──
    UPROPERTY(EditAnywhere, Category = "PT|UI|Skill")
    TObjectPtr<UDataTable> SkillDataTable;

private:
    UPROPERTY()
    TArray<TObjectPtr<UPTSkillEntryObject>> SkillEntries;

    FName SelectedSkillID = NAME_None;
};
