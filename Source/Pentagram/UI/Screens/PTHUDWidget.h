// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTHUDWidget.generated.h"

class UPTPlayerStatusWidget;
class UPTSkillSlotWidget;

UCLASS()
class PENTAGRAM_API UPTHUDWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()


public:
    UPTHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
    //~ Begin UCommonActivatableWidget Interface
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;
    //~ End UCommonActivatableWidget Interface

protected:
    /** 좌상단 HP/MP/EXP 묶음. UMG에서 같은 이름의 자식 위젯이 있어야 바인딩됨. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTPlayerStatusWidget> PlayerStatus;

    /** 하단 스킬 슬롯(퀵슬롯). */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTSkillSlotWidget> SkillSlots;

    // 미니맵/퀵슬롯/버프바/크로스헤어 등은 필요해질 때 추가
};

