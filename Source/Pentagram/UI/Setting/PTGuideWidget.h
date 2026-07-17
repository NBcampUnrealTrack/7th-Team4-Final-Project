// PTGuideWidget.h
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTGuideWidget.generated.h"

/**
 * 시작가이드
 * 화면 클릭 한번에 비활성화되는 CommonActivatableWidget
 */
UCLASS()
class PENTAGRAM_API UPTGuideWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnActivated() override;

protected:
    virtual void NativeConstruct() override;

    // 클릭감지
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // 터치대응
    virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InTouchEvent) override;

    // CommonActivatableWidget interface
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // 뒤로가기키 무시여부 (가이드는 클릭으로만 닫음)
    UPROPERTY(EditDefaultsOnly, Category = "PT|Guide")
    bool bDismissOnAnyClick = true;

private:
    void DismissGuide();
};
