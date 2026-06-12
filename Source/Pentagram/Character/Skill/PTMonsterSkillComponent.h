#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/PTSkillComponent.h"
#include "PTMonsterSkillComponent.generated.h"

UCLASS()
class PENTAGRAM_API UPTMonsterSkillComponent : public UPTSkillComponent
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Monster|Combat")
    bool PerformBasicAttack();

    UFUNCTION(BlueprintPure, Category = "PT|Monster|Combat")
    bool CanAttack() const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Monster|Combat")
    FName BasicAttackRowName = FName("BasicAttack");
};
