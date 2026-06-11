#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTNPCDialogueWidget.generated.h"

class APTQuestNPCCharacter;
class UTextBlock;
class UVerticalBox;
class UPTQuestListEntryWidget;

UCLASS()
class PENTAGRAM_API UPTNPCDialogueWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void SetupDialogue(APTQuestNPCCharacter* InNPC);

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void SelectQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void RequestAcceptQuest();

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void RequestRewardQuest();

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void CloseDialogue();

protected:
    virtual bool NativeOnHandleBackAction() override;

    void BuildQuestList();
    void ClearQuestText();
    void OnQuestEntryClicked(FName QuestID);
    void RefreshQuestText();

    UPROPERTY(BlueprintReadOnly, Category = "PT|NPC")
    TObjectPtr<APTQuestNPCCharacter> TargetNPC;

    UPROPERTY(BlueprintReadOnly, Category = "PT|NPC")
    FName SelectedQuestID = NAME_None;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_QuestName;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Description;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_Objective;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> QuestList;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Quest")
    TSubclassOf<UPTQuestListEntryWidget> QuestEntryWidgetClass;
};
