#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTStatBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class APTBasePlayerState;

UCLASS()
class PENTAGRAM_API UPTStatBarWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // 오버라이드
    virtual void NativeDestruct() override;

    // 값 갱신
    UFUNCTION(BlueprintCallable, Category = "PT|UI|StatBar")
    virtual void SetValue(float Current, float Max);

    // 즉시 적용
    UFUNCTION(BlueprintCallable, Category = "PT|UI|StatBar")
    void SetValueInstant(float Current, float Max);

    void SetupPlayerState(class APTBasePlayerState* PS);

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // 표시 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|StatBar")
    void OnDisplayValueUpdated(float InDisplayCurrent, float InMaxValue, float Percent);

    virtual void BindToPlayerState(class APTBasePlayerState* PS) {}
    virtual void UnbindFromPlayerState(class APTBasePlayerState* PS) {}

private:
    void ApplyDisplay();

protected:
    // 게이지 바
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> PB_Bar;

    // 값 라벨
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Value;

    // 보간 속도
    UPROPERTY(EditAnywhere, Category = "PT|UI|StatBar")
    float InterpSpeed = 8.f;

    // 목표 값
    UPROPERTY(BlueprintReadOnly, Category = "PT|UI|StatBar")
    float TargetCurrent = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "PT|UI|StatBar")
    float MaxValue = 1.f;

    // 표시 값
    UPROPERTY(BlueprintReadOnly, Category = "PT|UI|StatBar")
    float DisplayCurrent = 0.f;

    UPROPERTY()
    TWeakObjectPtr<APTBasePlayerState> BoundPS;
};
