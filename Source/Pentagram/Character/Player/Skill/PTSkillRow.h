#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PTSkillRow.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
    Active,
    Passive
};

USTRUCT(BlueprintType)
struct FPTSkillRow : public FTableRowBase
{
    GENERATED_BODY()

    // 스킬 식별자
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    FName SkillID;

    // MP 소모량
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float MPCost = 0.f;

    // 쿨다운
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float Cooldown = 0.f;

    // 공격력 데미지 배율
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float DamageMultiplier = 1.f;

    /* 액티브, 패시브 구분 나중에 스킬 구현 시 추가
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    ESkillType SkillType = ESkillType::Active;*/

    // 스킬 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    TSoftObjectPtr<UAnimMontage> SkillMontage;
};
