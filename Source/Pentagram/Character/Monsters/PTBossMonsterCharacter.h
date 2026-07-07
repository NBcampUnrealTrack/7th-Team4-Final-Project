#pragma once

#include "CoreMinimal.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "PTBossMonsterCharacter.generated.h"

class UPTBossPatternComponent;
class APTBossRoomCenter;

UCLASS()
class PENTAGRAM_API APTBossMonsterCharacter : public APTMonsterCharacter
{
	GENERATED_BODY()

public:
    APTBossMonsterCharacter();

    virtual void PerformAttack() override;

    UFUNCTION(BlueprintPure, Category = "PT|Boss|Phase")
    int32 GetCurrentPhase() const;

    UFUNCTION(BlueprintPure, Category = "PT|Boss|Phase")
    float GetDamageMultiplierForPhase(int32 Phase) const;

    /**
     * 반환값이 null일 수 있음 (BP에서 AttackMontage 미설정 시).
     * 호출부에서 반드시 null 체크 후 사용할 것.
     */
    UAnimMontage* GetAttackMontageForPhase(int32 Phase) const;

    UPTBossPatternComponent* GetBossPatternComponent() const { return BossPatternComponent; }

    UPROPERTY(BlueprintAssignable, Category = "PT|Boss|UI")
    FPTOnBossPhaseChanged OnPhaseChanged;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
    TObjectPtr<UPTBossPatternComponent> BossPatternComponent;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PT|Boss|Room")
    TObjectPtr<APTBossRoomCenter> RoomCenterActor;

protected:
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual float StartAttack() override;
    virtual void StopAttack() override;
    virtual float GetAttackDamage() const override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Animation")
    TObjectPtr<UAnimMontage> EnragedAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Animation")
    TObjectPtr<UAnimMontage> BerserkAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Phase")
    float Phase1DamageMultiplier = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Phase")
    float Phase2DamageMultiplier = 2.5f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Phase")
    float Phase1HPThreshold = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Phase")
    float Phase2HPThreshold = 0.1f;
};
