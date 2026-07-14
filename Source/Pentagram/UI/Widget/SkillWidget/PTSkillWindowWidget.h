#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Pentagram/Character/Skill/PTSkillRow.h"
#include "Interface/PTUIContentBoundsInterface.h" // 실제 경로에 맞게 수정
#include "PTSkillWindowWidget.generated.h"

class UPTPlayerSkillComponent;
class UListView;
class UImage;
class UTextBlock;
class UPTSkillSlotWidget;
class UPTSkillEntryObject;
class UUserWidget;
class UWidget;

UCLASS()
class PENTAGRAM_API UPTSkillWindowWidget : public UCommonActivatableWidget, public IPTUIContentBoundsInterface
{
    GENERATED_BODY()

public:
    // 스킬 목록 다시 그리기
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void RefreshSkillList();

    FText GetTargetingModeDisplayText(ESkillTargetingMode Mode) const;
protected:
    // ── 오버라이드 ──
    virtual void NativeOnInitialized() override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

    // Img_Background(창 전체 배경) 기준으로 콘텐츠 영역 판정.
    virtual bool IsScreenPositionOverContent_Implementation(const FVector2D& ScreenPosition) const override;

    // 항목 위젯 생성 시점 (클릭 델리게이트 바인딩용)
    void HandleEntryWidgetGenerated(UUserWidget& EntryWidget);

    UFUNCTION()
    void HandleSkillEntryClicked(FName SkillID, FPTSkillRow SkillRow);

    UFUNCTION()
    void HandleSkillAssignRequested(int32 SlotIndex, FName SkillID);

    UFUNCTION()
    void HandleSkillLearned(FName SkillID);

    void ShowSkillDetail(const FPTSkillRow& Row);

    void ClearSkillDetail();

    UPTPlayerSkillComponent* GetSkillComponent() const;
protected:
    // ── 위젯 바인딩 ──
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

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> Img_Background;

    // ── 설정 ──
    UPROPERTY(EditAnywhere, Category = "PT|UI|Skill")
    TObjectPtr<UDataTable> SkillDataTable;

private:
    UPROPERTY()
    TArray<TObjectPtr<UPTSkillEntryObject>> SkillEntries;

    FName SelectedSkillID = NAME_None;
};
