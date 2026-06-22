#pragma once

#include "CoreMinimal.h"
#include "PTGameTypes.h"
#include "GameFramework/GameStateBase.h"
#include "UI/Data/PTDelegates.h"    // 로비 추가
#include "PTGameState.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);

UCLASS()
class PENTAGRAM_API APTGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 로비 추가
    virtual void AddPlayerState(APlayerState* PlayerState) override;
    virtual void RemovePlayerState(APlayerState* PlayerState) override;

    UFUNCTION(BlueprintCallable, Category = "PT|GameState")
    void SetCurrentPhase(EGamePhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "PT|GameState")
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }

    // 로비 추가
    void NotifyLobbyUpdated();
    int32 GetReadyCount() const;
    int32 GetPlayerCount() const { return PlayerArray.Num(); }

    UFUNCTION(BlueprintCallable, Category = "PT|GameState")
    void SetQuestDataTable(UDataTable* InQuestDataTable);

    UFUNCTION(BlueprintCallable, Category = "PT|GameState")
    void SetItemDataTable(UDataTable* InItemDataTable);

protected:
    UFUNCTION()
    void OnRep_CurrentPhase();

    UFUNCTION()
    void OnRep_QuestDataTable();

    UFUNCTION()
    void OnRep_ItemDataTable();

    void OnGamePhaseChanged();
    void ApplyQuestDataTable() const;
    void ApplyItemDataTable() const;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase)
    EGamePhase CurrentPhase = EGamePhase::Waiting;

    UPROPERTY(ReplicatedUsing = OnRep_QuestDataTable, VisibleAnywhere, Category = "PT|Quest")
    TObjectPtr<UDataTable> QuestDataTable;

    UPROPERTY(ReplicatedUsing = OnRep_ItemDataTable, VisibleAnywhere, Category = "PT|Item")
    TObjectPtr<UDataTable> ItemDataTable;

public:
    // 로비 갱신 신호 로비 추가
    UPROPERTY(BlueprintAssignable, Category = "PT|Lobby")
    FPTOnLobbyUpdated OnLobbyUpdated;
    
    UPROPERTY(BlueprintAssignable, Category = "PT|GameState")
    FOnGamePhaseChanged OnGamePhaseChangedEvent;
};
