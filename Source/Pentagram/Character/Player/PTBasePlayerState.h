#pragma once

#include "CoreMinimal.h"
#include "Core/PTQuestDataRow.h"
#include "GameFramework/PlayerState.h"
#include "UI/Data/PTDelegates.h"
#include "PTBasePlayerState.generated.h"

UCLASS()
class PENTAGRAM_API APTBasePlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    // 모든 스탯 델리게이트를 한 번에 브로드캐스트 (UI 초기화용)
    UFUNCTION(BlueprintCallable, Category = "PT|Delegates")
    void BroadcastAllStats();

    // 리스폰 지점 세팅 및 반환
    UFUNCTION(BlueprintCallable, Category = "PT|Respawn")
    void SetSavedRespawnLocation(const FVector& NewLocation);

    UFUNCTION(BlueprintPure, Category = "PT|Respawn")
    FVector GetSavedRespawnLocation() const { return SavedRespawnLocation; }

    UFUNCTION(BlueprintPure, Category = "PT|Respawn")
    bool HasRespawnLocation() const { return bHasRespawnLocation; }

    // 준비 상태 변경 (서버 전용) 로비 추가
    void SetReady(bool bNewReady);
    // 로비 추가
    bool IsReady() const { return bIsReady; }

protected:
    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    // ── RepNotify 함수 ───────────────────────────────────────────────────────

    UFUNCTION()
    void OnRep_CurrentHP();

    UFUNCTION()
    void OnRep_MaxHP();

    UFUNCTION()
    void OnRep_CurrentMP();

    UFUNCTION()
    void OnRep_MaxMP();

    UFUNCTION()
    void OnRep_CurrentGold();

    UFUNCTION()
    void OnRep_CurrentExp();

    UFUNCTION()
    void OnRep_RequiredExp();

    UFUNCTION()
    void OnRep_PlayerLevel();

    // 로비 추가
    UFUNCTION()
    void OnRep_IsReady();
    UFUNCTION()
    void OnRep_AcceptedQuests();

public:
    // ── 멤버 변수 ────────────────────────────────────────────────────────────

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, VisibleAnywhere, Category = "PT|Stat")
    float CurrentHP;

    UPROPERTY(ReplicatedUsing = OnRep_MaxHP, VisibleAnywhere, Category = "PT|Stat")
    float MaxHP;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentMP, VisibleAnywhere, Category = "PT|Stat")
    float CurrentMP;

    UPROPERTY(ReplicatedUsing = OnRep_MaxMP, VisibleAnywhere, Category = "PT|Stat")
    float MaxMP;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentGold, VisibleAnywhere, Category = "PT|Economy")
    int32 CurrentGold = 0;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentExp, VisibleAnywhere, Category = "PT|Progress")
    int32 CurrentExp = 0;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerLevel, VisibleAnywhere, Category = "PT|Progress")
    int32 PlayerLevel = 1;

    UPROPERTY(ReplicatedUsing = OnRep_RequiredExp, VisibleAnywhere, Category = "PT|PlayerState|Progress")
    int32 RequiredExp = 100;

    // 로비 추가
    UPROPERTY(ReplicatedUsing = OnRep_IsReady, VisibleAnywhere, Category = "PT|Lobby")
    bool bIsReady = false;
    UPROPERTY(ReplicatedUsing = OnRep_AcceptedQuests, VisibleAnywhere, Category = "PT|Quest")
    TArray<FPTQuestProgress> AcceptedQuests;

private:
    // ── 멤버 변수 (private) ──────────────────────────────────────────────────

    // 실제 리스폰 위치 데이터가 저장될 곳 (서버에서만 안전하게 관리)
    UPROPERTY()
    FVector SavedRespawnLocation;

    UPROPERTY()
    bool bHasRespawnLocation = false;

public:
    // ── 델리게이트 (최하단) ──────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "PlayerState|Delegates")
    FPTOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "PlayerState|Delegates")
    FPTOnManaChanged OnManaChanged;

    UPROPERTY(BlueprintAssignable, Category = "PlayerState|Delegates")
    FPTOnLevelChanged OnLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "PlayerState|Delegates")
    FPTOnExpChanged OnExpChanged;

    UPROPERTY(BlueprintAssignable, Category = "PlayerState|Delegates")
    FPTOnGoldChanged OnGoldChanged;
};
