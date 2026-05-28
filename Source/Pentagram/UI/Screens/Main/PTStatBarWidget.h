// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTStatBarWidget.generated.h"

class UProgressBar;
class UTextBlock;


UCLASS()
class PENTAGRAM_API UPTStatBarWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    /** 외부에서 값을 갱신할 때 호출. (Component 델리게이트가 여기 바인딩됨) */
    UFUNCTION(BlueprintCallable, Category = "PT|UI|StatBar")
    virtual void SetValue(float Current, float Max);

    /** 즉시(보간 없이) 적용 — 초기화나 부활 시 사용. */
    UFUNCTION(BlueprintCallable, Category = "PT|UI|StatBar")
    void SetValueInstant(float Current, float Max);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** BP에서 텍스트 포맷 등을 커스터마이즈할 수 있게 분리. */
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|StatBar")
    void OnDisplayValueUpdated(float InDisplayCurrent, float InMaxValue, float Percent);

protected:
    /** UMG에서 BindWidget. ProgressBar 이름을 PB_Bar로 통일. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UProgressBar> PB_Bar;

    /** 선택적: "120 / 200" 같은 라벨. 없어도 됨(BindWidgetOptional). */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Value;

    /** 보간 속도(높을수록 빨리 따라옴). 0 이하면 즉시 적용. */
    UPROPERTY(EditAnywhere, Category = "PT|UI|StatBar")
    float InterpSpeed = 8.f;

    /** 실제 게임 값. */
    UPROPERTY(BlueprintReadOnly, Category = "PT|UI|StatBar")
    float TargetCurrent = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "PT|UI|StatBar")
    float MaxValue = 1.f;

    /** 화면에 그려지는 값(보간 결과). */
    UPROPERTY(BlueprintReadOnly, Category = "PT|UI|StatBar")
    float DisplayCurrent = 0.f;

private:
    void ApplyDisplay();
};
