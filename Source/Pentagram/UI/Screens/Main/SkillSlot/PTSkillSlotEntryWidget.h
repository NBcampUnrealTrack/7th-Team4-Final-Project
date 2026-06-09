#pragma once
#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTSkillSlotEntryWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTSkillSlotEntryWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
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

protected:
    // 오버라이드
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // BP 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|Skill")
    void OnCooldownUpdated(float Percent, float RemainingSeconds);

    // BP 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|Skill")
    void OnUsableChanged(bool bUsable);

private:
    // UI 갱신
    void ApplyCooldown(float Remaining);

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

private:
    float CooldownDuration = 0.f;
    float CooldownEndTime  = 0.f;
    bool  bOnCooldown      = false;
};
