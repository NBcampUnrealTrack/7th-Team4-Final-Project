#pragma once

#include "CoreMinimal.h"
#include "Character/PTBaseCharacter.h"
#include "PTMonsterState.h"
#include "PTMonsterCharacter.generated.h"

UCLASS()
class PENTAGRAM_API APTMonsterCharacter : public APTBaseCharacter
{
	GENERATED_BODY()

public:
    APTMonsterCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    bool bIsBoss = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    EMonsterState CurrentState = EMonsterState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|Monster")
    FName RowKey = NAME_None;

public:
    UFUNCTION(BlueprintCallable, Category = "PT|Monster")
    void InitializeMonster();

    UFUNCTION(BlueprintCallable, Category = "PT|Monster")
    void SetMonsterState(EMonsterState NewState);

protected:
    virtual void OnDeath();
};
