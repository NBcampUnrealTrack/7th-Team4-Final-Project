#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTQuestListEntryWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FPTOnQuestEntryClicked, FName);

class UButton;
class UTextBlock;

UCLASS()
class PENTAGRAM_API UPTQuestListEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetupQuestEntry(FName InQuestID, const FText& InQuestName);

    FPTOnQuestEntryClicked OnQuestEntryClicked;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void OnEntryClicked();

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_QuestEntry;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_QuestName;

    FName QuestID = NAME_None;
};
