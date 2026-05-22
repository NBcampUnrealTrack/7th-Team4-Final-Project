#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PTBasePlayerState.generated.h"

UCLASS()
class PENTAGRAM_API APTBasePlayerState : public APlayerState
{
	GENERATED_BODY()

public:

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, VisibleAnywhere, Category ="Stat")
    float CurrentHP;

    UPROPERTY(Replicated, VisibleAnywhere, Category ="Stat")
    float MaxHP;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentMP, VisibleAnywhere, Category ="Stat")
    float CurrentMP;

    UPROPERTY(Replicated, VisibleAnywhere, Category ="Stat")
    float MaxMP;

protected:

    UFUNCTION()
    void OnRep_CurrentHP();

    UFUNCTION()
    void OnRep_CurrentMP();

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
