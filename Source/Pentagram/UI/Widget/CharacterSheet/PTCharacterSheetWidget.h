// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interface/PTUIContentBoundsInterface.h" // 실제 경로에 맞게 수정
#include "PTCharacterSheetWidget.generated.h"

class APTBasePlayerState;
class UTextBlock;

/**
 * 캐릭터 시트 창.
 * HP/MP/공격력/방어력/치명타/이동속도 전부 PlayerState 델리게이트로 수신.
 * 창 열릴 때마다 PS->BroadcastAllStats()로 재브로드캐스트해서 리프레시.
 * 아이템 보너스 적용부는 장비 스탯 시스템 완성 전까지 0으로 고정.
 */
UCLASS()
class PENTAGRAM_API UPTCharacterSheetWidget : public UCommonActivatableWidget, public IPTUIContentBoundsInterface
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeOnActivated() override;

    // ── 위젯 바인딩 (WBP에서 이름 정확히 일치시킬 것) ──

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Name;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Level;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_HP;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_MP;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Atk;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Def;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_CritChance;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_CritDamage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_MoveSpeed;

    // 있으면 이걸 콘텐츠 영역 판정 기준으로 씀 (텍스트 바운딩박스보다 정확함).
    // WBP에 배경 보더/패널이 있으면 이 이름으로 바인딩할 것 - 없어도 컴파일은 됨(Optional).
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidget> Border_Background;

private:
    void TryBindFromOwningPlayerState();
    void BindToPlayerState(APTBasePlayerState* InPS);
    void UnbindFromPlayerState();

    // 바인딩된 텍스트 위젯들의 바운딩 박스를 합쳐서 "실제 콘텐츠 영역"으로 판정.
    // 배경 패널을 따로 안 만들어도 지금 있는 정보만으로 동작함.
    virtual bool IsScreenPositionOverContent_Implementation(const FVector2D& ScreenPosition) const override;

    UFUNCTION()
    void HandleHealthChanged(float CurrentHP, float MaxHP);

    UFUNCTION()
    void HandleManaChanged(float CurrentMP, float MaxMP);

    UFUNCTION()
    void HandleAttackChanged(float NewAttack);

    UFUNCTION()
    void HandleDefenseChanged(float NewDefense);

    UFUNCTION()
    void HandleCriticalChanged(float CriticalChance, float CriticalDamage);

    UFUNCTION()
    void HandleMoveSpeedChanged(float NewMoveSpeed);

    UFUNCTION()
    void HandleLevelChanged(int32 NewLevel);

    void RefreshPlayerName();


    FText FormatStatWithItemBonus(float BaseValue, float ItemBonus) const;

    TWeakObjectPtr<APTBasePlayerState> BoundPlayerState;
};
