#include "PTDamageNumberWidget.h"


void UPTDamageNumberWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPTDamageNumberWidget::InitDamageNumber(float DamageAmount, bool bIsCritical)
{
    if (!DamageAmount) return;

    DamageText->SetText(FText::AsNumber(FMath::RoundToInt(DamageAmount)));

    if (bIsCritical)
    {
        DamageText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.55f, 0.f, 1.f)));
        DamageText->SetRenderScale(FVector2D(1.6f, 1.6f));
    }
    else
    {
        DamageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        DamageText->SetRenderScale(FVector2D(1.f, 1.f));
    }

    ElapsedTime  = 0.f;
    bIsAnimating = true;
}

void UPTDamageNumberWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsAnimating) return;

    ElapsedTime += InDeltaTime;

    if (ElapsedTime >= FadeStartTime)
    {
        float Alpha = 1.f - ((ElapsedTime - FadeStartTime) / (LifeTime - FadeStartTime));
        Alpha =  FMath::Clamp(Alpha, 0.f, 1.f);

        if (DamageText)
        {
            FLinearColor CurrentColor = DamageText->GetColorAndOpacity().GetSpecifiedColor();
            CurrentColor.A = Alpha;
            DamageText->SetColorAndOpacity(FSlateColor(CurrentColor));
        }
    }

    if (ElapsedTime >= LifeTime)
    {
        bIsAnimating = false;
        RemoveFromParent();
    }
}

void UPTDamageNumberWidget::StartFadeOut()
{
}
