#include "Character/NPC/PTNPCCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"

APTNPCCharacter::APTNPCCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    SetRootComponent(MeshComponent);

    InteractionRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRangeSphere"));
    InteractionRangeSphere->SetupAttachment(MeshComponent);
    InteractionRangeSphere->SetSphereRadius(InteractionRadius);
    InteractionRangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void APTNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    InteractionRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &APTNPCCharacter::OnInteractionRangeBeginOverlap);
    InteractionRangeSphere->OnComponentEndOverlap.AddDynamic(this, &APTNPCCharacter::OnInteractionRangeEndOverlap);
}

void APTNPCCharacter::Interact(APlayerController* InstigatorController)
{
    if (!InstigatorController || !IsAvailableForInteraction())
    {
        return;
    }

    SetNPCState(ENPCState::Talking);
    OnDialogueStarted.Broadcast(InstigatorController);
}

void APTNPCCharacter::ServerInteract_Implementation(APlayerController* InstigatorController)
{

}

FName APTNPCCharacter::GetQuestID() const
{
    return QuestID;
}

void APTNPCCharacter::EndDialogue()
{
    SetNPCState(ENPCState::Idle);
    OnDialogueEnded.Broadcast();
}

void APTNPCCharacter::OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerController* PC = OtherActor ? OtherActor->GetInstigatorController<APlayerController>() : nullptr;
    if (PC)
    {
        OnPlayerEnterRange.Broadcast(PC);
    }
}

void APTNPCCharacter::OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerController* PC = OtherActor ? OtherActor->GetInstigatorController<APlayerController>() : nullptr;
    if (PC)
    {
        OnPlayerExitRange.Broadcast(PC);
    }
}

void APTNPCCharacter::SetNPCState(ENPCState NewState)
{
    CurrentState = NewState;
}
