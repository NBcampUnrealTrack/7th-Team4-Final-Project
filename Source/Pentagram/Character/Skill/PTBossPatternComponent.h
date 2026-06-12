#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Skill/PTSkillRow.h"
#include "PTBossPatternComponent.generated.h"

class UDataTable;
class UPTMonsterSkillComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PENTAGRAM_API UPTBossPatternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTBossPatternComponent();

	UFUNCTION(BlueprintCallable, Category = "PT|Boss|Pattern")
	void PreloadAllSkills();

	UFUNCTION(BlueprintCallable, Category = "PT|Boss|Pattern")
	void ExecuteSkillForPhase(int32 Phase);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
	TObjectPtr<UDataTable> BossSkillDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
	TArray<FName> Phase1SkillRowNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Boss|Pattern")
	TArray<FName> Phase2SkillRowNames;

	UPROPERTY()
	TObjectPtr<UPTMonsterSkillComponent> SkillComponent;

protected:
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    TPair<FName, FPTBossSkillRow*> PickNextSkill(int32 Phase);

    TMap<FName, FTimerHandle> PatternCooldownTimers;
    TMap<FName, bool>         PatternCooldownFlags;

    bool bSkillAssetsLoaded = false;
};
