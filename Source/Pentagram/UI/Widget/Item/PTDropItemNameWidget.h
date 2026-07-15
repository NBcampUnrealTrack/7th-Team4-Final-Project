#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTDropItemNameWidget.generated.h"

class UTextBlock;

/**
 * 필드에 드랍된 아이템의 이름을 표시합니다.
 * WBP가 지정되지 않아도 기본 텍스트 위젯을 생성하며, WBP 파생 클래스에서는
 * Txt_ItemName을 배치해 외형을 교체할 수 있습니다.
 */
UCLASS()
class PENTAGRAM_API UPTDropItemNameWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Item|Drop")
    void SetItemName(const FText& InItemName);

    UFUNCTION(BlueprintPure, Category = "PT|Item|Drop")
    FText GetItemName() const { return DisplayItemName; }

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativePreConstruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Drop", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_ItemName;

private:
    void RefreshItemName();

    UPROPERTY(Transient)
    FText DisplayItemName;
};
