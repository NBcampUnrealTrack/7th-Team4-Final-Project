#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "PTDamageNumberWidget.generated.h"

UCLASS()
class PENTAGRAM_API UPTDamageNumberWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitDamageNumber(float DamageAmount, bool bIsCritical);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DamageText;

private:
    void StartFadeOut();

    float ElapsedTime   = 0.f;
    float LifeTime      = 1.2f;
    float FadeStartTime = 0.7f;
    bool bIsAnimating   = false;
};
