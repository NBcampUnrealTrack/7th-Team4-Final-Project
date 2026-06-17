#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/PTCharacterRow.h"
#include "Character/PTCombatTypes.h"
#include "PTBaseCharacter.generated.h"

UCLASS()
class PENTAGRAM_API APTBaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APTBaseCharacter();

    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    void PostInitializeComponents();

    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 데미지 적용
    UFUNCTION(BlueprintCallable, Category = "PT|Combat")
    virtual float ApplyDamage(float DamageAmount, AActor* Attacker);

    UFUNCTION(BlueprintCallable, Category = "PT|Combat")
    virtual float ApplyDamageWithHit(float DamageAmount, AActor* Attacker, const FPTHitInfo& HitInfo);

    // 사망 처리
    UFUNCTION(BlueprintCallable, Category = "PT|Combat")
    virtual void OnDeath();

    UFUNCTION()
    virtual void OnRep_CurrentHP();

    float GetAttackSpeed() const { return AttackSpeed; }

protected:
    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    void ApplyHit(const FPTHitInfo& HitInfo);
    void ApplyHitStop(float Duration);
    void RestoreHitStop();
    void ApplyKnockback(const FPTHitInfo& HitInfo);

public:
    // ── 멤버 변수 ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Character")
    ECharacterType CharacterType;

    UPROPERTY(EditAnywhere, Category = "PT|Data")
    FDataTableRowHandle CharacterDataHandle;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, VisibleAnywhere, Category = "PT|Stats")
    float CurrentHP;    // 현재 체력

    UPROPERTY(Replicated, VisibleAnywhere, Category = "PT|Stats")
    float MaxHP;        // 최대 체력

    UPROPERTY(Replicated, VisibleAnywhere, Category = "PT|Stats")
    float CurrentMP;    // 현재 마나

    UPROPERTY(Replicated, VisibleAnywhere, Category = "PT|Stats")
    float MaxMP;        // 최대 마나

    UPROPERTY(Replicated, VisibleAnywhere, Category = "PT|Stats")
    float BaseDef;      // 방어력

    UPROPERTY(Replicated, VisibleAnywhere, Category = "PT|Stats")
    float BaseAtk;      // 공격력

    UPROPERTY(Replicated, VisibleAnywhere, Category = "PT|Stats")
    float AttackSpeed;  // 공격 속도

    UPROPERTY(Replicated, VisibleAnywhere, Category = "PT|Stats")
    float MoveSpeed;    // 이동 속도

    UPROPERTY(EditAnywhere, Category = "PT|Hit")
    TObjectPtr<UAnimMontage> HitReaction_Light;

    UPROPERTY(EditAnywhere, Category = "PT|Hit")
    TObjectPtr<UAnimMontage> HitReaction_Heavy;

    UPROPERTY(BlueprintReadOnly, Category = "PT|Hit")
    bool bIsStaggered = false;

private:
    float DefaultTimeDilation = 1.f;

    FTimerHandle HitStopTimer;
    FTimerHandle StaggerTimer;
};
