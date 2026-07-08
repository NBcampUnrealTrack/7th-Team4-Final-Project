#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTChatWidget.generated.h"

class UScrollBox;
class UEditableText;

UCLASS()
class PENTAGRAM_API UPTChatWidget : public UCommonUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void HandleChatMessageReceived(FString SenderName, FString Message);

    UFUNCTION()
    void OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitType);

    void AppendChatRow(const FString& SenderName, const FString& Message);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> ChatScrollBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableText> ChatInputBox;
};
