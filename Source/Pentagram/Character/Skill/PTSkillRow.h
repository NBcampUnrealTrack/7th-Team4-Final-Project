#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    FName SkillID;

    // MP 소모량
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    float MPCost = 0.f;

    // 쿨다운
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    float Cooldown = 0.f;

    // 공격력 데미지 배율
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    float DamageMultiplier = 1.f;

    /* 액티브, 패시브 구분 나중에 스킬 구현 시 추가
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    ESkillType SkillType = ESkillType::Active;*/

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    float SkillRadius = 100.f;

    // 스킬 발생 위치 오프셋 (캐릭터를 중심으로 기준)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    FVector SkillOffset = FVector(0.f, 0.f, 0.f); //앞, 양옆, 위아래

    // 스킬 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    TSoftObjectPtr<UAnimMontage> SkillMontage;

    // 스킬 시전 사운드
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    TSoftObjectPtr<USoundBase> SkillSound;

    // 타격 지점 사운드
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    TSoftObjectPtr<USoundBase> SkillHitSound;

    // 스킬 발동 시 실행할 이펙트
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    TSoftObjectPtr<UNiagaraSystem> SkillEffect;
};

USTRUCT(BlueprintType)
struct FPTBossSkillRow : public FPTSkillRow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    float Weight = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    float PatternCooldown = 3.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Skill")
    TSoftObjectPtr<UAnimMontage> OverrideMontage;
};
