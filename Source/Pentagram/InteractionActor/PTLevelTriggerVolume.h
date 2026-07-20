#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTLevelTriggerVolume.generated.h"

class UBoxComponent;
class APTMonsterCharacter;
class APTPlayerCharacter;

UCLASS()
class PENTAGRAM_API APTLevelTriggerVolume : public AActor
{
    GENERATED_BODY()

public:
    APTLevelTriggerVolume();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnTargetMonsterDied(APTMonsterCharacter* DeadMonster);

    void CheckClearCondition();
    void ExecuteLevelTransition();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> TriggerVolume;

    // 이 트리거에서 처치 대상인 몬스터들 - 레벨에 배치된 인스턴스를 직접 드래그해서 연결
    UPROPERTY(EditInstanceOnly, Category = "PT|LevelTrigger")
    TArray<TObjectPtr<APTMonsterCharacter>> TargetMonsters;

    // 다음에 이동할 레벨 - 5개 레벨 전부 이 값만 다르게 세팅
    UPROPERTY(EditInstanceOnly, Category = "PT|LevelTrigger")
    FName NextLevelName;

    // 조건 만족 후 트랜지션까지 딜레이 (연출용)
    UPROPERTY(EditAnywhere, Category = "PT|LevelTrigger")
    float TransitionDelay = 1.5f;

private:
    UPROPERTY()
    TSet<TObjectPtr<APTPlayerCharacter>> PlayersInVolume;

    int32 DeadMonsterCount = 0;
    bool bTriggered = false;
    FTimerHandle TransitionTimerHandle;
};
