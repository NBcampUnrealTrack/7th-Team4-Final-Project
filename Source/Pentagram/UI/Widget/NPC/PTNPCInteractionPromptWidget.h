#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTNPCInteractionPromptWidget.generated.h"

class APTNPCCharacter;

UCLASS()
class PENTAGRAM_API UPTNPCInteractionPromptWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    UPTNPCInteractionPromptWidget(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void SetupPrompt(APTNPCCharacter* InNPC);

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void ShowPrompt();

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void HidePrompt();

protected:
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "PT|NPC")
    TObjectPtr<APTNPCCharacter> TargetNPC;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC")
    FVector PromptWorldOffset = FVector(0.f, 0.f, 140.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC")
    FVector2D PromptScreenOffset = FVector2D(60.f, -20.f);

private:
    void UpdatePromptPosition();
};
