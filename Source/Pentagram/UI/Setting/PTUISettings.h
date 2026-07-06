#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UI/Manage/PTUIManagerSubsystem.h"   // EPTUILayer
#include "PTUISettings.generated.h"

class UCommonActivatableWidget;
class UPTNotifyManagerWidget;
class UPTLoadingWidget;
class UTexture2D;

USTRUCT(BlueprintType)
struct FPTUILevelEntry
{
    GENERATED_BODY()

    // UI 위젯
    UPROPERTY(EditAnywhere, Category = "PT|UI")
    TSoftClassPtr<UCommonActivatableWidget> WidgetClass;

    // 레이어
    UPROPERTY(EditAnywhere, Category = "PT|UI")
    EPTUILayer Layer = EPTUILayer::GameMenu;

    // 스트림 여부
    UPROPERTY(EditAnywhere, Category = "PT|UI")
    bool bIsStreamingLevel = true;

    // 이 레벨에서 쓸 알림 위젯. 비워두면(None) 알림 기능 자체가 없는 레벨
    UPROPERTY(EditAnywhere, Category = "PT|UI|Notify")
    TSoftClassPtr<UPTNotifyManagerWidget> NotifyWidgetClass;

    UPROPERTY(EditAnywhere, Category = "PT|UI|Notify")
    FText ZoneDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|UI")
    bool bAllowGameplayUI = false;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PT UI Settings"))
class PENTAGRAM_API UPTUISettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const override { return TEXT("Game"); }

    // 레벨 -> UI 매핑
    UPROPERTY(Config, EditAnywhere, Category = "PT|UI")
    TMap<FName, FPTUILevelEntry> LevelUITable;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Loading")
    TSoftClassPtr<UPTLoadingWidget> LoadingWidgetClass;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Loading")
    TArray<TSoftObjectPtr<UTexture2D>> LoadingBackgroundImages;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Loading")
    bool bRandomizeLoadingBackground = true;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Loading", meta = (ClampMin = "0.1"))
    float LoadingBackgroundCycleInterval = 3.f;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Loading")
    TArray<FString> LoadingTips;

    UPROPERTY(Config, EditAnywhere, Category = "PT|Loading", meta = (ClampMin = "0"))
    int32 LoadingWidgetZOrder = 10000;
};
