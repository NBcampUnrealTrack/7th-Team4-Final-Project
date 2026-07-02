// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTPartyFrameWidget.generated.h"

class UPanelWidget;
class UPTPartySlotWidget;
class APTBasePlayerState;

/**
 *
 */
UCLASS()
class PENTAGRAM_API UPTPartyFrameWidget : public UCommonUserWidget
{
    GENERATED_BODY()

protected:
    // 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 파티 갱신
    void RefreshParty();

protected:
    // 슬롯 컨테이너
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UPanelWidget> Box_Slots;

    // 슬롯 클래스
    UPROPERTY(EditAnywhere, Category = "PT|UI|Party")
    TSubclassOf<UPTPartySlotWidget> SlotWidgetClass;

    // 본인 포함
    UPROPERTY(EditAnywhere, Category = "PT|UI|Party")
    bool bIncludeLocalPlayer = false;

    // 갱신 주기
    UPROPERTY(EditAnywhere, Category = "PT|UI|Party")
    float RefreshInterval = 0.5f;

    // 슬롯 간격
    UPROPERTY(EditAnywhere, Category = "PT|UI|Party")
    float SlotSpacing = 8.f;
private:
    // 슬롯 맵
    UPROPERTY(Transient)
    TMap<TObjectPtr<APTBasePlayerState>, TObjectPtr<UPTPartySlotWidget>> SlotMap;

    // 타이머
    FTimerHandle RefreshTimerHandle;
};
