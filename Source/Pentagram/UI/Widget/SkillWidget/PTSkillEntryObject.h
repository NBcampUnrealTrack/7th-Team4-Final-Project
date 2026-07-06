#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Pentagram/Character/Skill/PTSkillRow.h"
#include "PTSkillEntryObject.generated.h"

// 스킬 리스트 한 줄에 대응하는 데이터 객체
UCLASS()
class PENTAGRAM_API UPTSkillEntryObject : public UObject
{
    GENERATED_BODY()

public:
    // 스킬 식별자
    UPROPERTY(BlueprintReadOnly, Category = "PT|Skill")
    FName SkillID = NAME_None;

    // 스킬 원본 데이터
    UPROPERTY(BlueprintReadOnly, Category = "PT|Skill")
    FPTSkillRow SkillRow;
};
