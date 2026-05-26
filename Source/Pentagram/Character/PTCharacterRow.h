#pragma once

#include "Engine/DataTable.h"
#include "PTCharacterRow.generated.h"

USTRUCT(BlueprintType)
struct FPTCharacterRow : public FTableRowBase
{
    GENERATED_BODY()

    // ── 공통 스탯 ( 플레이어 / 몬스터 / 보스 ) ──────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Stats")
    float MaxHP = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Stats")
    float BaseAtk = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Stats")
    float BaseDef = 0.f;

    // AnimNotify 기반 재생 비율. 1.0 = 원본 속도
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Stats")
    float AttackSpeed = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Stats")
    float MoveSpeed = 0.f;

    // ── 플레이어 전용 (몬스터 미사용 → 0) ────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Player")
    float MaxMp = 0.f;

    // 크리티컬 데미지 배율. 2.0 = 200%. 몬스터 미사용 → 0
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Player")
    float CriticalATK = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Player")
    float CriticalChance = 0.f;

    // 다음 레벨업에 필요한 경험치.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Player")
    int32 RequiredEXP = 0;

    // ── 몬스터 전용 AI (플레이어 미사용 → 0) ─────────────────────────────

    // AIPerception 시야 각도 (도 단위). 쫄몹 140 / 보스 180
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|AI")
    float SightAngle = 0.f;

    // 플레이어 최초 감지 거리 (유닛). 쫄몹 800 / 보스 1000
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|AI")
    float SightRange = 0.f;

    // 추격 유지 거리 (유닛). 초과 시 Chase 해제. 쫄몹 1200 / 보스 1500
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|AI")
    float ChaseRange = 0.f;

    // 공격 판정 거리 (유닛). 쫄몹 150 / 보스 300
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|AI")
    float AttackRange = 0.f;

    // 순찰 반경 (유닛). 스폰 위치 기준. 쫄몹·보스 공통 1000
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|AI")
    float PatrolRadius = 0.f;

    // 스폰 위치 기준 최대 이탈 거리 (유닛). 초과 시 강제 Patrol 복귀
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|AI")
    float MaxChaseDistance = 0.f;

    // ── 몬스터 전용 드롭·보상 (플레이어 미사용 → 0) ──────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|Drop")
    int32 GoldDropMin = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|Drop")
    int32 GoldDropMax = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|Drop")
    float EquipDropRate = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character|Monster|Drop")
    int32 RewardExp = 0;
};
