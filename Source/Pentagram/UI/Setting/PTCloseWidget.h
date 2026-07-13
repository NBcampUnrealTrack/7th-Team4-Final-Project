// PTCloseWidget.h
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTCloseWidget.generated.h"

class UButton;

/**
 * 종료 확인 위젯
 * 화면을 덮는 모달 형태로 동작 (종료 버튼 + 닫기 버튼)
 */
UCLASS()
class PENTAGRAM_API UPTCloseWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UPTCloseWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

    // 딤배경 클릭시 닫기 여부
    UPROPERTY(EditDefaultsOnly, Category = "PT|Modal")
    bool bCloseOnOutsideClick = true;

    // 게임 종료
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Quit;

    // 이 위젯 닫기
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Close;

    // 딤 배경 버튼(옵션, 배경 전체를 덮는 투명 버튼)
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> DimBackgroundButton;

private:
    UFUNCTION()
    void HandleQuitClicked();

    UFUNCTION()
    void HandleCloseClicked();
};
