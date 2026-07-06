#pragma once
#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTSkillSlotEntryWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UDragDropOperation;

// 스킬 드롭됨
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTOnSkillEntryDropped, FName, SkillID);

UCLASS()
class PENTAGRAM_API UPTSkillSlotEntryWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // ── 델리게이트 ──
    UPROPERTY(BlueprintAssignable, Category = "PT|UI|Skill")
    FPTOnSkillEntryDropped OnSkillDropped;

    // 아이콘 설정
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void SetIcon(UTexture2D* Icon);

    // 쿨다운 시작
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void StartCooldown(float Duration);

    // 남은시간 지정
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void SetCooldownRemaining(float Remaining, float Duration);

    // 사용가능 설정
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void SetUsable(bool bUsable);

    // 단축키 설정
    UFUNCTION(BlueprintCallable, Category = "PT|UI|Skill")
    void SetKeyLabel(const FText& Key);

    // 슬롯 인덱스
    void SetSlotIndex(int32 InIndex) { SlotIndex = InIndex; }
    int32 GetSlotIndex() const { return SlotIndex; }

protected:
    // 오버라이드
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    // BP 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|Skill")
    void OnCooldownUpdated(float Percent, float RemainingSeconds);

    // BP 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|Skill")
    void OnUsableChanged(bool bUsable);

private:
    // UI 갱신
    void ApplyCooldown(float Remaining);
    void UpdateHighlight(bool bIsOver);

public:
    // 기본 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PT|UI|Skill")
    TObjectPtr<UTexture2D> DefaultIcon;

    // 기본 단축키
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PT|UI|Skill")
    FText KeyText = FText::FromString(TEXT("Q"));

    // 아이콘 색상
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PT|UI|Skill")
    FLinearColor IconTint = FLinearColor::White;

    // 드롭 허용 색
    UPROPERTY(EditAnywhere, Category = "PT|UI|Skill")
    FLinearColor DropHighlightColor = FLinearColor(1.f, 0.85f, 0.2f, 0.5f);

protected:
    // UI 이미지
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_Icon;

    // UI 게이지
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> PB_Cooldown;

    // UI 남은초
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Cooldown;

    // UI 단축키
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Key;

    // 드롭 하이라이트 (원형 테두리 이미지)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_DropHighlight;

private:
    float CooldownDuration = 0.f;
    float CooldownEndTime  = 0.f;
    bool  bOnCooldown      = false;
    int32 SlotIndex        = INDEX_NONE;
};
