// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTPartySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UPTPartyHealthBar;
class UPTPartyManaBar;
class APTBasePlayerState;

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTPartySlotWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // 슬롯 설정
    void SetupSlot(APTBasePlayerState* PS);

    // 슬롯 해제
    void ClearSlot();

    // 대상 조회
    APTBasePlayerState* GetBoundPlayerState() const { return BoundPS.Get(); }

protected:
    // 오버라이드
    virtual void NativeDestruct() override;

    // 레벨 변경
    UFUNCTION()
    void HandleLevelChanged(int32 NewLevel);

protected:
    // 초상화
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_Portrait;

    // 클래스 아이콘
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_ClassIcon;

    // 이름
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Name;

    // 레벨
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Level;

    // 체력 바
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTPartyHealthBar> HealthBar;

    // 마나 바
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTPartyManaBar> ManaBar;

private:
    // 대상 PS
    TWeakObjectPtr<APTBasePlayerState> BoundPS;
};
