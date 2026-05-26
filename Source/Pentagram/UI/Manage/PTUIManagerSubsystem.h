// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTUIManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EPTUILayer : uint8
{
    HUD         UMETA(DisplayName = "HUD"),       // GameLayer
    GameMenu    UMETA(DisplayName = "Game Menu"), // MenuLayer
    Modal       UMETA(DisplayName = "Modal"),     // ModalLayer
};

class UPTPrimaryLayout;
class UPTActivatableWidgetBase;
class UCommonActivatableWidget;

UCLASS()
class PENTAGRAM_API UPTUIManagerSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    UPTUIManagerSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** APTHUD에서 PrimaryLayout 생성 직후 호출. 매니저가 레이어에 접근할 수 있게 한다. */
    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void RegisterPrimaryLayout(UPTPrimaryLayout* InLayout);

    /** 지정 레이어의 스택에 위젯을 푸시. 스택이라 같은 레이어에 여러 개 쌓이고 LIFO로 동작. */
    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    UPTActivatableWidgetBase* PushWidget(TSubclassOf<UPTActivatableWidgetBase> WidgetClass, EPTUILayer Layer);

    /** 위젯을 스택에서 제거(Deactivate → 자동 pop). */
    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void RemoveWidget(UPTActivatableWidgetBase* WidgetToRemove);

    /** 디버그/조회용. */
    UPTPrimaryLayout* GetPrimaryLayout() const { return PrimaryLayout.Get(); }

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<UPTPrimaryLayout> PrimaryLayout;
};
