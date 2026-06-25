// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTStatBarWidget.h"
#include "UI/Data/PTDelegates.h"
#include "PTExpBarWidget.generated.h"

class APTBasePlayerState;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTExpBarWidget : public UPTStatBarWidget
{
    GENERATED_BODY()

public:
    // EXP 콜백
    UFUNCTION()
    void HandleExpChanged(float Current, float Required);

    // 레벨 콜백
    UFUNCTION()
    void HandleLevelChanged(int32 NewLevel);

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void BindToPlayerState(APTBasePlayerState* PS) override;
    virtual void UnbindFromPlayerState(APTBasePlayerState* PS) override;

    // 레벨 텍스트 갱신
    void UpdateLevelText(int32 NewLevel);

    // 레벨업 연출
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|UI|StatBar")
    void OnLevelUpVisual(int32 NewLevel);

protected:
    // 위젯
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Level;
};
