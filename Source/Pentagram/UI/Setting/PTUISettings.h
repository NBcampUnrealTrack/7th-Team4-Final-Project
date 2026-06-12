#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UI/Manage/PTUIManagerSubsystem.h"   // EPTUILayer
#include "PTUISettings.generated.h"

class UCommonActivatableWidget;

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
