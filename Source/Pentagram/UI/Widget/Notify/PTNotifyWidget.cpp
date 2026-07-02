// PTNotifyWidget.cpp
#include "PTNotifyWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "TimerManager.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UPTNotifyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Anim_In)
	{
		FWidgetAnimationDynamicEvent InFinishedEvent;
		InFinishedEvent.BindDynamic(this, &UPTNotifyWidget::HandleInAnimFinished);
		BindToAnimationFinished(Anim_In, InFinishedEvent);
	}

	if (Anim_Out)
	{
		FWidgetAnimationDynamicEvent OutFinishedEvent;
		OutFinishedEvent.BindDynamic(this, &UPTNotifyWidget::HandleOutAnimFinished);
		BindToAnimationFinished(Anim_Out, OutFinishedEvent);
	}
}

void UPTNotifyWidget::PlayNotify(const FPTNotifyData& InData)
{
	UE_LOG(LogTemp, Warning, TEXT("[Notify][6] PlayNotify called. IsDead=%d bIsShowing=%d"),
		IsOwningPlayerDead(), bIsShowing);

	if (IsOwningPlayerDead())
	{
		return;
	}

	if (bIsShowing)
	{
		return;
	}

	bIsShowing = true;
	CurrentData = InData;

	if (Txt_Message)
	{
		Txt_Message->SetText(CurrentData.Message);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Notify][6] FAILED - Txt_Message is null (BindWidget 이름 불일치 의심)"));
	}

	OnApplyNotifyStyle(CurrentData.Type);

	//사운드 재생
	PlayNotifySound(CurrentData.Type);

	if (Anim_In)
	{
		PlayAnimation(Anim_In);
	}
	else
	{
		HandleInAnimFinished();
	}
}

void UPTNotifyWidget::HandleInAnimFinished()
{
	GetWorld()->GetTimerManager().SetTimer(
		DurationTimerHandle,
		this,
		&UPTNotifyWidget::PlayOutAnim,
		CurrentData.Duration,
		false);
}

void UPTNotifyWidget::PlayOutAnim()
{
	if (Anim_Out)
	{
		PlayAnimation(Anim_Out);
	}
	else
	{
		HandleOutAnimFinished();
	}
}

void UPTNotifyWidget::HandleOutAnimFinished()
{

	bIsShowing = false;
	OnNotifyFinished.Broadcast();
}

bool UPTNotifyWidget::IsOwningPlayerDead() const
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return false;

    APTBasePlayerState* PS = PC->GetPlayerState<APTBasePlayerState>();
    if (!PS) return false;

    return PS->CurrentHP <= 0.f;
}

void UPTNotifyWidget::PlayNotifySound(EPTNotifyType InType) const
{
	const TObjectPtr<USoundBase>* FoundSound = TypeSounds.Find(InType);
	if (!FoundSound || !(*FoundSound))
	{
		return;
	}

	UGameplayStatics::PlaySound2D(this, *FoundSound);
}
