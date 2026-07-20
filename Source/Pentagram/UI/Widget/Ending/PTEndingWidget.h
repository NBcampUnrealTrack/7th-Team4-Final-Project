// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTEndingWidget.generated.h"

class UCommonTextBlock;
class UButton;

UCLASS()
class PENTAGRAM_API UPTEndingWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    // 엔딩 타이틀
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Txt_EndingTitle;

    // 엔딩 내용
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Txt_EndingBody;

    // 인트로 이동 버튼
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_ReturnToIntro;

    // 타이틀 문구
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ending")
    FText EndingTitle = FText::FromString(TEXT("탈출 성공"));

    // 본문 문구
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ending")
    FText EndingBody = FText::FromString(TEXT("당신은 지하감옥을 벗어나 숲으로 나왔다"));

    // 이동할 레벨명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ending")
    FName IntroLevelName = TEXT("L_MainMenu");

private:
    UFUNCTION()
    void HandleReturnToIntroClicked();
};
