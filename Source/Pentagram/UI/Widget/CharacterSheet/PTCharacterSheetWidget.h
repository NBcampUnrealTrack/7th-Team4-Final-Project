// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTCharacterSheetWidget.generated.h"

class APTBasePlayerState;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTCharacterSheetWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeOnActivated() override;

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

private:
    void TryBindFromOwningPlayerState();
    void BindToPlayerState(APTBasePlayerState* InPS);
    void UnbindFromPlayerState();

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

    // 기본값 + 아이템 보너스 표기. 보너스 0이면 괄호 생략
    FText FormatStatWithItemBonus(float BaseValue, float ItemBonus) const;

    TWeakObjectPtr<APTBasePlayerState> BoundPlayerState;
};
