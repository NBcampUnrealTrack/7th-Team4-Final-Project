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
<<<<<<< HEAD:Source/Pentagram/UI/Screens/Main/PTHUDWidget.h
    // 오버라이드
=======
>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion"):Source/Pentagram/UI/Screens/Main/Player/PTHUDWidget.h
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    virtual bool NativeOnHandleBackAction() override;

    // HP/MP/EXP 묶음
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTPlayerStatusWidget> PlayerStatus;

<<<<<<< HEAD:Source/Pentagram/UI/Screens/Main/PTHUDWidget.h
    // 스킬 슬롯
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTSkillSlotWidget> SkillSlots;
=======
  // 하단 스킬 슬롯(퀵슬롯)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPTSkillSlotWidget> SkillSlots;

>>>>>>> parent of 2fcee51 (Revert "Merge branch 'develop' into feature/gamemode/gamestate-conversion"):Source/Pentagram/UI/Screens/Main/Player/PTHUDWidget.h
};
