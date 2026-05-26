// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PTHUD.generated.h"

class UPTPrimaryLayout;
class UPTActivatableWidgetBase;
UCLASS()
class PENTAGRAM_API APTHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
    /** 블루프린트에서 WBP_PrimaryLayout 할당 */
    UPROPERTY(EditDefaultsOnly, Category = "PT|UI|HUD")
    TSubclassOf<UPTPrimaryLayout> MainLayoutClass;

    /** 게임 시작 시 자동으로 GameLayer에 푸시할 기본 HUD 위젯 (예: WBP_PT_PlayerHUD) */
    UPROPERTY(EditDefaultsOnly, Category = "PT|UI|HUD")
    TSubclassOf<UPTActivatableWidgetBase> InitialHUDWidgetClass;

    /** 생성된 레이아웃 추적 */
    UPROPERTY(Transient)
    TObjectPtr<UPTPrimaryLayout> MainLayoutWidget;
};
