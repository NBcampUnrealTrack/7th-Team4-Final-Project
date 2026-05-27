// Fill out your copyright notice in the Description page of Project Settings.

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


    // 서버에서 호출. CurrentPhase 갱신 후 OnRep 경유로 클라이언트에 전파됨.
    UFUNCTION(BlueprintCallable, Category = "PT|GameState")
    void SetCurrentPhase(EGamePhase NewPhase);      //페이즈 설정

    UFUNCTION(BlueprintPure, Category = "PT|GameState")
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }     //페이즈 가져오기

    UPROPERTY(Replicated)
    int32 ElapsedTime= 0;          // 게임 진행 시간

protected:

    UFUNCTION()
    void OnRep_CurrentPhase();

    void OnGamePhaseChanged();  // 상태 변경 시 UI 이벤트 브로드캐스트

    UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase)
    EGamePhase CurrentPhase = EGamePhase::Waiting;    // 게임 상태 (Waiting, InProgress, GameOver)
};
