#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTNPCCharacter.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class APlayerController;

UENUM(BlueprintType)
enum class ENPCState : uint8
{
    Idle    UMETA(DisplayName = "Idle"),
    Talking UMETA(DisplayName = "Talking"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTNPCPlayerControllerDelegate, APlayerController*, PlayerController);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTNPCSimpleDelegate);

UCLASS()
class PENTAGRAM_API APTNPCCharacter : public AActor
{
    GENERATED_BODY()

public:
    APTNPCCharacter();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void Interact(APlayerController* InstigatorController);

    UFUNCTION(Server, Reliable)
    void ServerInteract(APlayerController* InstigatorController);

    UFUNCTION(BlueprintCallable, Category = "PT|NPC")
    void EndDialogue();

    UFUNCTION(BlueprintPure, Category = "PT|NPC")
    ENPCState GetNPCState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "PT|NPC")
    bool IsAvailableForInteraction() const { return CurrentState == ENPCState::Idle; }

    FName GetQuestID() const;

    // 인터랙션 UI 표시 연동 (F키 안내 표시)
    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Interaction")
    FPTNPCPlayerControllerDelegate OnPlayerEnterRange;

    // 인터랙션 UI 숨김 연동
    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Interaction")
    FPTNPCPlayerControllerDelegate OnPlayerExitRange;

    // 다이얼로그 연동: 대화 시작 시 브로드캐스트
    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Dialogue")
    FPTNPCPlayerControllerDelegate OnDialogueStarted;

    // 다이얼로그 연동: 대화 종료 시 브로드캐스트
    UPROPERTY(BlueprintAssignable, Category = "PT|NPC|Dialogue")
    FPTNPCSimpleDelegate OnDialogueEnded;

protected:
    UFUNCTION()
    void OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PT|Components")
    TObjectPtr<USphereComponent> InteractionRangeSphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PT|NPC|Quest")
    FName QuestID = NAME_None;

private:
    void SetNPCState(ENPCState NewState);

    UPROPERTY(VisibleAnywhere, Category = "PT|State")
    ENPCState CurrentState = ENPCState::Idle;

    static constexpr float InteractionRadius = 200.f;
};
