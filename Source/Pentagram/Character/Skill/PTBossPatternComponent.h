#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Skill/PTSkillRow.h"
#include "PTBossPatternComponent.generated.h"

class UDataTable;
class UPTMonsterSkillComponent;
class APTBossProjectile;
class APTAreaWarning;
class UNiagaraSystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTBossPatternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTBossPatternComponent();

	UFUNCTION(BlueprintCallable, Category = "PT|Boss|Pattern")
	void PreloadAllSkills();

	UFUNCTION(BlueprintCallable, Category = "PT|Boss|Pattern")
	float ExecuteSkillForPhase(int32 Phase);

    UFUNCTION(BlueprintCallable, Category = "PT|Boss|Pattern")
    void ExecutePendingSkill();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSpawnAreaWarningBatch(const TArray<FVector>& DropLocations, float BaseDelay, float Interval, UNiagaraSystem* FallEffect, UNiagaraSystem* ImpactEffect, float StartHeight, TSubclassOf<APTAreaWarning> WarningClass, float MaxRadius);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSpawnAreaFX(FVector CastLocation, bool bHasSafeZone, FVector SafeZoneCenter, UNiagaraSystem* CastFX, UNiagaraSystem* SafeZoneFX, float SafeZoneRadius, float AreaAttackDelay);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
	TObjectPtr<UDataTable> BossSkillDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
	TArray<FName> Phase0SkillRowNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
	TArray<FName> Phase1SkillRowNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
	TArray<FName> Phase2SkillRowNames;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Pattern")
    TSubclassOf<APTBossProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Pattern")
    FName ProjectileSpawnSocket = NAME_None;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Pattern")
    float ProjectileSpawnForwardOffset = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "PT|Boss|Pattern")
    float ProjectileSpawnHeightOffset = 100.f;

	UPROPERTY()
	TObjectPtr<UPTMonsterSkillComponent> SkillComponent;

protected:
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void SpawnProjectile(const FPTBossSkillRow& RowSnapshot);
    void SpawnAreaAttack(const FPTBossSkillRow& RowSnapshot);

private:
    TPair<FName, FPTBossSkillRow*> PickNextSkill(int32 Phase);
    AActor* GetTargetActor() const;

    TMap<FName, FTimerHandle> PatternCooldownTimers;
    TMap<FName, bool>         PatternCooldownFlags;

    FPTBossSkillRow PendingSkillSnapshot;
    bool bHasPendingSkill = false;

    TArray<FTimerHandle> AreaAttackTimers;
    bool bSkillAssetsLoaded = false;

    bool bAreaAttackInProgress = false;
};
