// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "PTCommonButtonBase.generated.h"

UCLASS()
class PENTAGRAM_API UPTCommonButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

protected:
    // 에디터 프리뷰 및 초기화
    virtual void NativePreConstruct() override;

    // 입력 액션 위젯 상태 변경 시 호출
    virtual void UpdateInputActionWidget() override;

    // 입력 방식(키보드/패드) 변경 시 호출
    virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;

    // 블루프린트에서 실제 텍스트 컴포넌트를 변경할 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "PT Button")
    void UpdateButtonText(const FText& InText);

    // 블루프린트에서 버튼의 시각적 스타일(색상, 테두리 등)을 갱신할 이벤트 (추가됨!)
    UFUNCTION(BlueprintImplementableEvent, Category = "PT Button")
    void UpdateButtonStyle();

    // 외부에서 수동으로 텍스트를 설정하는 함수
    UFUNCTION(BlueprintCallable, Category = "PT Button")
    void SetButtonText(const FText& InText);

    // 조건에 따라 텍스트를 선택하여 갱신
    void RefreshButtonText();

    // 기본 버튼 텍스트
    UPROPERTY(EditAnywhere, Category = "PT Button")
    FText ButtonText;

    // 텍스트 강제 덮어쓰기 여부
    bool bOverride_ButtonText;
};
