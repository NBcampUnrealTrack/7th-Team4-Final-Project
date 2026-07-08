#pragma once

#include "CoreMinimal.h"
#include "PTGameTypes.h"
#include "GameFramework/GameStateBase.h"
#include "UI/Data/PTDelegates.h"    // 로비 추가
#include "PTGameState.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);

// 로비 채팅 추가
USTRUCT(BlueprintType)
struct FPTChatLogEntry
{
    GENERATED_BODY()

    UPROPERTY()
    FString SenderName;

    UPROPERTY()
    FString Message;
};

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

    // 로비 채팅 추가
    void Server_AddChatMessage(const FString& SenderName, const FString& Message);

    // 로비 채팅 추가
    UPROPERTY(ReplicatedUsing = OnRep_ChatLog)
    TArray<FPTChatLogEntry> ChatLog;
protected:
    UFUNCTION()
    void OnRep_CurrentPhase();

    UFUNCTION()
    void OnRep_QuestDataTable();

    UFUNCTION()
    void OnRep_ItemDataTable();

    // 로비 채팅 추가
    UFUNCTION()
    void OnRep_ChatLog();

    void OnGamePhaseChanged();
    void ApplyQuestDataTable() const;
    void ApplyItemDataTable() const;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase)
    EGamePhase CurrentPhase = EGamePhase::Waiting;

    UPROPERTY(ReplicatedUsing = OnRep_QuestDataTable, VisibleAnywhere, Category = "PT|Quest")
    TObjectPtr<UDataTable> QuestDataTable;

    UPROPERTY(ReplicatedUsing = OnRep_ItemDataTable, VisibleAnywhere, Category = "PT|Item")
    TObjectPtr<UDataTable> ItemDataTable;

private:
    // 로비 채팅 추가
    int32 LastBroadcastChatIndex = 0;
    static constexpr int32 MaxChatLogSize = 100;

public:
    // 로비 갱신 신호 로비 추가
    UPROPERTY(BlueprintAssignable, Category = "PT|Lobby")
    FPTOnLobbyUpdated OnLobbyUpdated;

    // 로비 채팅 추가
    UPROPERTY(BlueprintAssignable, Category = "PT|Lobby")
    FPTOnChatMessageReceived OnChatMessageReceived;

    UPROPERTY(BlueprintAssignable, Category = "PT|GameState")
    FOnGamePhaseChanged OnGamePhaseChangedEvent;
};
