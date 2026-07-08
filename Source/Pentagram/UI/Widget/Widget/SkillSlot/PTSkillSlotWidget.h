#pragma once
#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTSkillSlotWidget.generated.h"

class UPTSkillSlotEntryWidget;
class UPTPlayerSkillComponent;

// 슬롯 배정 요청 (인덱스 + 스킬ID)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTOnSkillSlotAssignRequested, int32, SlotIndex, FName, SkillID);

UCLASS()
class PENTAGRAM_API UPTSkillSlotWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // ── 델리게이트 ──
    UPROPERTY(BlueprintAssignable, Category = "PT|UI|Skill")
    FPTOnSkillSlotAssignRequested OnSkillAssignRequested;

    // 아이콘 변경
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void SetSlotIcon(int32 SlotIndex, UTexture2D* Icon);

    // 쿨다운 시작
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void StartSlotCooldown(int32 SlotIndex, float Duration);

    // 사용가능 설정
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void SetSlotUsable(int32 SlotIndex, bool bUsable);

    // 컴포넌트 연결
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void InitWithSkillComponent(UPTPlayerSkillComponent* InSkillComp);

    void RefreshUsability();

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 쿨다운 시작
    UFUNCTION()
    void HandleCooldownStart(int32 SlotIndex, float Duration);

    // 쿨다운 종료
    UFUNCTION()
    void HandleCooldownEnd(int32 SlotIndex);

    // 슬롯 배정됨 (신규)
    UFUNCTION()
    void HandleSlotAssigned(int32 SlotIndex, FName SkillID);

    // 슬롯별 드롭 핸들러 (동적 델리게이트는 인자로 슬롯 구분 불가해 개별 바인딩)
    UFUNCTION()
    void HandleSlotQDropped(FName SkillID);
    UFUNCTION()
    void HandleSlotWDropped(FName SkillID);
    UFUNCTION()
    void HandleSlotEDropped(FName SkillID);
    UFUNCTION()
    void HandleSlotRDropped(FName SkillID);

private:
    // 슬롯 찾기
    UPTSkillSlotEntryWidget* GetEntry(int32 SlotIndex) const;

protected:
    // 슬롯 Q
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTSkillSlotEntryWidget> Slot_Q;

    // 슬롯 W
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTSkillSlotEntryWidget> Slot_W;

    // 슬롯 E
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTSkillSlotEntryWidget> Slot_E;

    // 슬롯 R
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UPTSkillSlotEntryWidget> Slot_R;

private:
    // 슬롯 배열
    UPROPERTY()
    TArray<TObjectPtr<UPTSkillSlotEntryWidget>> Entries;

    UPROPERTY()
    TWeakObjectPtr<UPTPlayerSkillComponent> SkillComp;
};
