#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/EngineTypes.h" // FTimerHandle
#include "PTChatWidget.generated.h"

class UScrollBox;
class UEditableText;
class APTGameState;

UCLASS()
class PENTAGRAM_API UPTChatWidget : public UCommonUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // GameState 리플리케이션 대기 후 바인딩 (클라이언트는 Construct 시점에 GameState가 아직 없을 수 있음)
    void BindGameState();

    UFUNCTION()
    void HandleChatMessageReceived(FString SenderName, FString Message);

    UFUNCTION()
    void OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitType);

    void AppendChatRow(const FString& SenderName, const FString& Message);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> ChatScrollBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableText> ChatInputBox;

private:
    FTimerHandle BindRetryTimer;

    UPROPERTY()
    TObjectPtr<APTGameState> BoundGameState; // 델리게이트 언바인드용 캐시
};
