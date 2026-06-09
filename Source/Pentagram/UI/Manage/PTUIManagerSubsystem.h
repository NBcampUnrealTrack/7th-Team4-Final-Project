#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
//#include  "UI/Data/PTDelegates.h"
#include "PTUIManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EPTUILayer : uint8
{
    HUD         UMETA(DisplayName = "HUD"),       // 게임
    GameMenu    UMETA(DisplayName = "Game Menu"), // 메뉴
    Modal       UMETA(DisplayName = "Modal"),     // 모달
};

class UPTPrimaryLayout;
class UPTHUDWidget;
class UCommonActivatableWidget;

UCLASS()
class PENTAGRAM_API UPTUIManagerSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    UPTUIManagerSubsystem();

    // 오버라이드
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // 레이아웃 등록
    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void RegisterPrimaryLayout(UPTPrimaryLayout* InLayout);

    // 위젯 푸시
    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    UCommonActivatableWidget* PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass, EPTUILayer Layer);

    // 위젯 제거
    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void RemoveWidget(UCommonActivatableWidget* WidgetToRemove);

    // 인벤 토글
    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void ToggleInventory(TSubclassOf<UCommonActivatableWidget> InventoryClass);

    // 레이아웃 조회
    UPTPrimaryLayout* GetPrimaryLayout() const { return PrimaryLayout.Get(); }

protected:
    // 인벤 핸들
    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> InventoryInstance;

private:
    // 베이스 레이아웃
    UPROPERTY(Transient)
    TWeakObjectPtr<UPTPrimaryLayout> PrimaryLayout;
};
