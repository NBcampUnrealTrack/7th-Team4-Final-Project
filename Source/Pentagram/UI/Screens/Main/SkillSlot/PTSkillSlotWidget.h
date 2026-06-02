#pragma once
#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTSkillSlotWidget.generated.h"

class UPTSkillSlotEntryWidget;

UCLASS()
class PENTAGRAM_API UPTSkillSlotWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // 아이콘 변경
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void SetSlotIcon(int32 SlotIndex, UTexture2D* Icon);

    // 쿨다운 시작
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void StartSlotCooldown(int32 SlotIndex, float Duration);

    // 사용가능 설정
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void SetSlotUsable(int32 SlotIndex, bool bUsable);

protected:
    virtual void NativeConstruct() override;

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
    // 슬롯 찾기
    UPTSkillSlotEntryWidget* GetEntry(int32 SlotIndex) const;

    // 슬롯 배열
    UPROPERTY()
    TArray<TObjectPtr<UPTSkillSlotEntryWidget>> Entries;
};
