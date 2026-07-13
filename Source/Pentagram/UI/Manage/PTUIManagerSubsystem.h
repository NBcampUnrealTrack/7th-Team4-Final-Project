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
struct FPTUILevelEntry;
struct FPTNotifyData;
class UPTNotifyManagerWidget;
class UPTPrimaryLayout;
class UPTHUDWidget;
class UCommonActivatableWidget;
class UPTNPCDialogueWidget;

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

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    UCommonActivatableWidget* OpenInventoryForShop(TSubclassOf<UCommonActivatableWidget> InventoryClass);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void CloseShopInventory();

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void ToggleQuest(TSubclassOf<UPTNPCDialogueWidget> QuestClass);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void ToggleCloseWidget(TSubclassOf<UCommonActivatableWidget> CloseWidgetClass);

    // 레이아웃 조회
    UPTPrimaryLayout* GetPrimaryLayout() const { return PrimaryLayout.Get(); }

    UFUNCTION(BlueprintCallable, Category = "PT|UI|Notify")
    void ShowNotify(const FPTNotifyData& InData);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void ToggleSkillWindow(TSubclassOf<UCommonActivatableWidget> SkillWindowClass);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    void ToggleCharacterSheet(TSubclassOf<UCommonActivatableWidget> CharacterSheetClass);

    UFUNCTION(BlueprintCallable, Category = "PT|UI")
    bool IsScreenPositionOverGameplayUI(const FVector2D& ScreenPosition) const;

protected:
    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> InventoryInstance;

    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> ShopInstance;

    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> ShopInventoryInstance;

    UPROPERTY(Transient)
    TObjectPtr<UPTNPCDialogueWidget> QuestInstance;

    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> CloseWidgetInstance;
    // 현재 UI
    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> CurrentUIWidget;

    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> SkillWindowInstance;

    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> CharacterSheetInstance;
private:
    void SetupNotifyWidgetForLevel(const FPTUILevelEntry& InEntry);

    // 게임플레이 UI오픈 가능 여부 체크
    bool CanOpenGameplayUI() const { return bAllowGameplayUI; }

    // 게임플레이 UI를 전부 닫음.

    void CloseAllGameplayUI();

    UPROPERTY(Transient)
    TWeakObjectPtr<UPTPrimaryLayout> PrimaryLayout;

    // 현재 스트림 레벨
    FName CurrentStreamLevelName;

    bool bAllowGameplayUI = false;

    UPROPERTY()
    TObjectPtr<UPTNotifyManagerWidget> CurrentNotifyWidget;
};
