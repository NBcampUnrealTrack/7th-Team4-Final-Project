// PTNotifyTypes.h
#pragma once

#include "CoreMinimal.h"
#include "PTNotifyTypes.generated.h"

// 알림 종류
UENUM(BlueprintType)
enum class EPTNotifyType : uint8
{
    LevelUp,
    ZoneEnter,
    Quest,

};

USTRUCT(BlueprintType)
struct FPTNotifyData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    EPTNotifyType Type = EPTNotifyType::LevelUp;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FText Message;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    float Duration = 2.f;
};
