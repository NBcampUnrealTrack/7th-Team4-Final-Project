// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTEndingTrigger.generated.h"

class UCommonActivatableWidget;

UCLASS()
class PENTAGRAM_API APTEndingTrigger : public AActor
{
    GENERATED_BODY()

public:
    APTEndingTrigger();

protected:
    virtual void BeginPlay() override;

    // 몇 초 뒤에 띄울지
    UPROPERTY(EditAnywhere, Category = "Ending")
    float DelaySeconds = 3.0f;

    // 띄울 엔딩 위젯 클래스
    UPROPERTY(EditAnywhere, Category = "Ending")
    TSubclassOf<UCommonActivatableWidget> EndingWidgetClass;

private:
    void ShowEndingWidget();

    FTimerHandle EndingTimerHandle;
};
