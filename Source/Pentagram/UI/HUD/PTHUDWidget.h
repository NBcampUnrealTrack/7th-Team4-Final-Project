// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTHUDWidget.generated.h"

class UPTPlayerStatusWidget;
class UPTSkillSlotWidget;
class UPTMonsterHealthBarWidget;
class UButton;
UCLASS()
class PENTAGRAM_API UPTHUDWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:

    UPTHUDWidget(const FObjectInitializer& ObjectInitializer);


    //몬스터 타겟 확인
    UFUNCTION()
    void HandleMonsterTargeted(AActor* TargetMonster);

    UFUNCTION(BlueprintCallable, Category = "PT|Online")
    void OpenSteamInviteUI();
protected:
    // 오버라이드
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

    // HP/MP/EXP 묶음
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTPlayerStatusWidget> PlayerStatus;

    // 스킬 슬롯
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTSkillSlotWidget> SkillSlots;

    // 몬스터 체크
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTMonsterHealthBarWidget> MonsterTargetFrame;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> InviteButton;
};
