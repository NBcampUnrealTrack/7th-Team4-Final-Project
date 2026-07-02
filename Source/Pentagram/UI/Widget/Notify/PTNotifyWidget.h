// PTNotifyWidget.h
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTNotifyTypes.h"
#include "UI/Data/PTDelegates.h"
#include "PTNotifyWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;
class USoundBase;

// 실제 화면에 뜨는 알림 슬롯 위젯 (재사용, 타입별 분기는 UMG에서)
UCLASS()
class PENTAGRAM_API UPTNotifyWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // 매니저가 호출하는 진입점
    UFUNCTION(BlueprintCallable)
    void PlayNotify(const FPTNotifyData& InData);

    UPROPERTY(BlueprintAssignable)
    FPTOnNotifyFinished OnNotifyFinished;

protected:
    virtual void NativeOnInitialized() override;

    // 자식(UMG)에서 타입별 스타일 분기용으로 오버라이드
    UFUNCTION(BlueprintImplementableEvent)
    void OnApplyNotifyStyle(EPTNotifyType InType);

    UFUNCTION()
    void HandleInAnimFinished();

    UFUNCTION()
    void HandleOutAnimFinished();

    UFUNCTION()
    void PlayOutAnim();

    // 방어코드 1: 플레이어 사망 상태면 알림을 띄우지 않음
    // TODO: 실제 사망 판정 방식에 맞춰 구현 채우기 (PlayerState / StatComponent 등)
    virtual bool IsOwningPlayerDead() const;

    // 타입별 사운드 재생 (없으면 재생 안 함)
    void PlayNotifySound(EPTNotifyType InType) const;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Message;

    // UMG에서 In/Out 애니메이션 각각 준비 (이름 일치 필수)
    UPROPERTY(Transient, meta = (BindWidgetAnim), BlueprintReadOnly)
    TObjectPtr<UWidgetAnimation> Anim_In;

    UPROPERTY(Transient, meta = (BindWidgetAnim), BlueprintReadOnly)
    TObjectPtr<UWidgetAnimation> Anim_Out;

    // 타입별 사운드. WBP_PTNotifySlot의 Class Defaults 패널에서 지정
    UPROPERTY(EditDefaultsOnly, Category = "PT|UI|Notify")
    TMap<EPTNotifyType, TObjectPtr<USoundBase>> TypeSounds;

private:
    FTimerHandle DurationTimerHandle;

    FPTNotifyData CurrentData;

    // 방어코드 2: 현재 알림이 재생 중인지 (중복 재생 방지, 사운드 중복 재생도 같이 막힘)
    bool bIsShowing = false;
};
