#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UI/Manage/PTUIManagerSubsystem.h"   // EPTUILayer
#include "PTUISettings.generated.h"

class UCommonActivatableWidget;
class UPTNotifyManagerWidget;

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
    // (예: L_Intro, L_MainMenu)
    UPROPERTY(EditAnywhere, Category = "PT|UI|Notify")
    TSoftClassPtr<UPTNotifyManagerWidget> NotifyWidgetClass;

    // 레벨 진입 시 자동으로 띄울 지역명. 비워두면 지역 알림 표시 안 함
    // (레벨업 알림은 이 필드와 무관하게 게임플레이 코드에서 직접 호출)
    UPROPERTY(EditAnywhere, Category = "PT|UI|Notify")
    FText ZoneDisplayName;
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
};
