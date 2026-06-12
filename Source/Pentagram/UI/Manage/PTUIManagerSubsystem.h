#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTUIManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EPTUILayer : uint8
{
    HUD         UMETA(DisplayName = "HUD"),
    GameMenu    UMETA(DisplayName = "Game Menu"),
    Modal       UMETA(DisplayName = "Modal"),
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

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void RegisterPrimaryLayout(UPTPrimaryLayout* InLayout);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    UCommonActivatableWidget* PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass, EPTUILayer Layer);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void RemoveWidget(UCommonActivatableWidget* WidgetToRemove);

    // 레벨 UI 열기
    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void OpenUILevel(FName LevelName);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void ToggleInventory(TSubclassOf<UCommonActivatableWidget> InventoryClass);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void ToggleShop(TSubclassOf<UCommonActivatableWidget> ShopClass);

    UPTPrimaryLayout* GetPrimaryLayout() const { return PrimaryLayout.Get(); }

protected:
    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> InventoryInstance;

    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> ShopInstance;

    // 현재 UI
    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> CurrentUIWidget;

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<UPTPrimaryLayout> PrimaryLayout;

    // 현재 스트림 레벨
    FName CurrentStreamLevelName;
};
