#include "PTGoldWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Character/Player/PTBasePlayerState.h"
#include "TimerManager.h"

void UPTGoldWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
    ApplyGoldIcon();
}

void UPTGoldWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BindToPlayerState();
}

void UPTGoldWidget::NativeDestruct()
{
    if (BoundState.IsValid())
    {
        BoundState->OnGoldChanged.RemoveDynamic(this, &UPTGoldWidget::HandleGoldChanged);
    }
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BindRetryTimer);
    }
    Super::NativeDestruct();
}

void UPTGoldWidget::BindToPlayerState()
{
    APTBasePlayerState* PS = Cast<APTBasePlayerState>(GetOwningPlayerState());
    if (!PS)
    {
        // 스테이트 대기
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                BindRetryTimer, this, &UPTGoldWidget::BindToPlayerState, 0.2f, false);
        }
        return;
    }

    BoundState = PS;
    PS->OnGoldChanged.AddUniqueDynamic(this, &UPTGoldWidget::HandleGoldChanged);

    // 초기 값
    PS->BroadcastAllStats();
}

void UPTGoldWidget::ApplyGoldIcon()
{
    if (Img_GoldIcon && GoldIcon)
    {
        Img_GoldIcon->SetBrushFromTexture(GoldIcon);
    }
}

void UPTGoldWidget::HandleGoldChanged(int64 NewAmount)
{
    if (Txt_Gold)
    {
        Txt_Gold->SetText(FText::AsNumber(NewAmount));
    }
}
