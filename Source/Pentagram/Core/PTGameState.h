#pragma once

#include "CoreMinimal.h"
#include "PTGameTypes.h"
#include "GameFramework/GameStateBase.h"
#include "PTGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);

UCLASS()
class PENTAGRAM_API APTGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "PT|GameState")
    void SetCurrentPhase(EGamePhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "PT|GameState")
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }

private:
    UPROPERTY(Replicated)
    int32 ElapsedTime= 0;

protected:
    UFUNCTION()
    void OnRep_CurrentPhase();

    void OnGamePhaseChanged();

    UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase)
    EGamePhase CurrentPhase = EGamePhase::Waiting;

public:
    UPROPERTY(BlueprintAssignable, Category = "PT|GameState")
    FOnGamePhaseChanged OnGamePhaseChangedEvent;
};
