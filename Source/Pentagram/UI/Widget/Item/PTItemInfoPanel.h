#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/PTItemTypes.h"
#include "PTItemInfoPanel.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTItemInfoPanel : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Item|Info")
    void SetItemData(const FItemData& InItemData);

    UFUNCTION(BlueprintCallable, Category = "PT|Item|Info")
    void ClearItemData();

    UFUNCTION(BlueprintPure, Category = "PT|Item|Info")
    const FItemData& GetItemData() const { return ItemData; }

    UFUNCTION(BlueprintPure, Category = "PT|Item|Info")
    bool HasItemData() const { return !ItemData.Item_ID.IsNone(); }

protected:
    virtual void NativeOnInitialized() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "PT|Item|Info")
    void OnRefreshItemInfo(const FItemData& InItemData);

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Info", meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_ItemPreview;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Info", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_ItemName;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Info", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_ItemType;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Info", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_ItemGrade;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Info", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_BaseStat;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Info", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Options;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Info", meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Description;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Item|Info")
    FItemData ItemData;

private:
    FText MakeItemTypeText() const;
    FText MakeItemGradeText() const;
    FText MakeBaseStatText() const;
    FText MakeOptionsText() const;
};
