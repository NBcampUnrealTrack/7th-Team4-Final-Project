// PTNotifyManagerWidget.h
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTNotifyTypes.h"
#include "PTNotifyManagerWidget.generated.h"

class UPTNotifyWidget;

// HUD에 배치되어 알림 큐를 순차 재생하는 매니저 위젯
// 게임플레이 코드는 이 위젯의 Enqueue만 호출하면 됨
UCLASS()
class PENTAGRAM_API UPTNotifyManagerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Notify")
	void Enqueue(const FPTNotifyData& InData);

protected:
	virtual void NativeOnInitialized() override;


private:
    void ProcessNext();

    UFUNCTION()
    void HandleCurrentFinished();
protected:
	// 실제 표시되는 알림 슬롯 위젯 (UMG에 1개 배치)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTNotifyWidget> Notify_Slot;

private:
	TQueue<FPTNotifyData> NotifyQueue;

	bool bIsPlaying = false;
};
