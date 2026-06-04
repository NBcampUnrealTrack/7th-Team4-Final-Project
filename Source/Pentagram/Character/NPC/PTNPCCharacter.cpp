#include "Character/NPC/PTNPCCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/PTQuestSubsystem.h"
#include "GameFramework/PlayerController.h"

APTNPCCharacter::APTNPCCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
    SetRootComponent(SceneRootComponent);

    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(SceneRootComponent);

    InteractionRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRangeSphere"));
    InteractionRangeSphere->SetupAttachment(SceneRootComponent);
    InteractionRangeSphere->SetSphereRadius(InteractionRadius);
    InteractionRangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void APTNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    InteractionRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &APTNPCCharacter::OnInteractionRangeBeginOverlap);
    InteractionRangeSphere->OnComponentEndOverlap.AddDynamic(this, &APTNPCCharacter::OnInteractionRangeEndOverlap);
}

void APTNPCCharacter::Interact(APlayerController* InteractPlayerController)
{
    if (!InteractPlayerController || !IsAvailableForInteraction())
    {
        return;
    }

    SetNPCState(ENPCState::Talking);
    OnDialogueStarted.Broadcast(InteractPlayerController);
}

void APTNPCCharacter::ServerInteract_Implementation(APlayerController* InteractPlayerController)
{
    if (InteractPlayerController == nullptr)
    {
        return;
    }

    if (NPCID.IsNone())
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    QuestSubsystem->UpdateQuestProgress(EPTQuestConditionType::TalkToNPC, NPCID);
}

void APTNPCCharacter::ServerAcceptQuest_Implementation(FName QuestID)
{
    if (QuestID.IsNone() || !QuestIDs.Contains(QuestID))
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem == nullptr)
    {
        return;
    }

    QuestSubsystem->AcceptQuest(QuestID);
}

FName APTNPCCharacter::GetNPCID() const
{
    return NPCID;
}

const TArray<FName>& APTNPCCharacter::GetQuestIDs() const
{
    return QuestIDs;
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
