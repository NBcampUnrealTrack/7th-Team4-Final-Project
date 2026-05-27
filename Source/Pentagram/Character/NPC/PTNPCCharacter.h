#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTNPCCharacter.generated.h"

class USphereComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class ENPCState : uint8
{
    Idle    UMETA(DisplayName = "Idle"),
    Talking UMETA(DisplayName = "Talking"),
};

UCLASS()
class PENTAGRAM_API APTNPCCharacter : public AActor
{
    GENERATED_BODY()

public:
    APTNPCCharacter();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void Interact(APlayerController* InstigatorController);

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void EndDialogue();

    UFUNCTION(BlueprintPure, Category = "PT|NPC")
    ENPCState GetNPCState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "PT|NPC")
    bool IsAvailableForInteraction() const { return CurrentState == ENPCState::Idle; }

protected:
    UFUNCTION()
    void OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Day 9: 인터랙션 UI 표시 연동 (F키 안내 표시)
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|NPC")
    void OnPlayerEnterRange(APlayerController* PlayerController);

    // Day 9: 인터랙션 UI 숨김 연동
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|NPC")
    void OnPlayerExitRange(APlayerController* PlayerController);

    // Day 4 (다이얼로그 연동): 대화 시작 시 호출
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|NPC")
    void OnDialogueStarted(APlayerController* InstigatorController);

    // Day 4 (다이얼로그 연동): 대화 종료 시 호출
    UFUNCTION(BlueprintImplementableEvent, Category = "PT|NPC")
    void OnDialogueEnded();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<USphereComponent> InteractionRangeSphere;

private:
    void SetNPCState(ENPCState NewState);

    UPROPERTY(VisibleAnywhere, Category = "PT|NPC|State")
    ENPCState CurrentState = ENPCState::Idle;

    static constexpr float InteractionRadius = 200.f;
};
