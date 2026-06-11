#pragma once

#include "CoreMinimal.h"
#include "UI/Components/PTCommonButtonBase.h"
#include "PTQuestListEntryWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FPTOnQuestEntryClicked, FName);

UCLASS()
class PENTAGRAM_API UPTQuestListEntryWidget : public UPTCommonButtonBase
{
    GENERATED_BODY()

public:
    void SetupQuestEntry(FName InQuestID, const FText& InQuestName);

    FPTOnQuestEntryClicked OnQuestEntryClicked;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void OnEntryClicked();

    FName QuestID = NAME_None;
};
