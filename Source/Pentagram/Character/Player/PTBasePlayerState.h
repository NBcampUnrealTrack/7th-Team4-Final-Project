#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PTBasePlayerState.generated.h"

UCLASS()
class PENTAGRAM_API APTBasePlayerState : public APlayerState
{
	GENERATED_BODY()

public:

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, VisibleAnywhere, Category ="PT|Stat")
    float CurrentHP;

    UPROPERTY(Replicated, VisibleAnywhere, Category ="PT|Stat")
    float MaxHP;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentMP, VisibleAnywhere, Category ="PT|Stat")
    float CurrentMP;

    UPROPERTY(Replicated, VisibleAnywhere, Category ="PT|Stat")
    float MaxMP;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentGold, VisibleAnywhere, Category = "PT|Economy")
    int32 CurrentGold = 0;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentExp, VisibleAnywhere, Category = "PT|Progress")
    int32 CurrentExp = 0;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerLevel, VisibleAnywhere, Category = "PT|Progress")
    int32 PlayerLevel = 1;

    UPROPERTY(ReplicatedUsing = OnRep_RequiredExp, VisibleAnywhere, Category = "PT|PlayerState|Progress")
    int32 RequiredExp = 100;

protected:

    UFUNCTION()
    void OnRep_CurrentHP();

    UFUNCTION()
    void OnRep_CurrentMP();

    UFUNCTION()
    void OnRep_RequiredExp();

    UFUNCTION()
    void OnRep_PlayerLevel();

    UFUNCTION()
    void OnRep_CurrentGold();

    UFUNCTION()
    void OnRep_CurrentExp();

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
