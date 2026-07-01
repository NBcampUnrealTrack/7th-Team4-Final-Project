#include "Character/NPC/PTNPCCharacter.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UI/Widget/NPC/PTNPCInteractionPromptWidget.h"

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

    InteractionPromptWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidgetComponent"));
    InteractionPromptWidgetComponent->SetupAttachment(SceneRootComponent);
    InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
    InteractionPromptWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    InteractionPromptWidgetComponent->SetDrawAtDesiredSize(true);
    InteractionPromptWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractionPromptWidgetComponent->SetGenerateOverlapEvents(false);
    InteractionPromptWidgetComponent->SetVisibility(false);
}

void APTNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    InteractionRangeSphere->SetSphereRadius(InteractionRadius);

    InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
    if (InteractionPromptWidgetClass != nullptr)
    {
        InteractionPromptWidgetComponent->SetWidgetClass(InteractionPromptWidgetClass);
    }
    InteractionPromptWidgetComponent->SetVisibility(false);

    InteractionRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &APTNPCCharacter::OnInteractionRangeBeginOverlap);
    InteractionRangeSphere->OnComponentEndOverlap.AddDynamic(this, &APTNPCCharacter::OnInteractionRangeEndOverlap);
}

void APTNPCCharacter::Interact_Implementation(AActor* InteractorCharacter)
{
    if (!HasAuthority() || InteractorCharacter == nullptr)
    {
        return;
    }

    APawn* InteractPawn = Cast<APawn>(InteractorCharacter);
    APlayerController* InteractPlayerController =
        InteractPawn ? Cast<APlayerController>(InteractPawn->GetController()) : nullptr;

    if (InteractPlayerController == nullptr)
    {
        return;
    }

    if (!NPCID.IsNone())
    {
        UGameInstance* GameInstance = GetGameInstance();
        UPTQuestSubsystem* QuestSubsystem =
            GameInstance != nullptr ? GameInstance->GetSubsystem<UPTQuestSubsystem>() : nullptr;
        APTBasePlayerState* PlayerState =
            InteractPlayerController->GetPlayerState<APTBasePlayerState>();
        if (QuestSubsystem != nullptr && PlayerState != nullptr)
        {
            QuestSubsystem->UpdateQuestProgress(
                PlayerState,
                EPTQuestConditionType::TalkToNPC,
                NPCID);
        }
    }

    OnDialogueStarted.Broadcast(InteractPlayerController);
}

FName APTNPCCharacter::GetNPCID() const
{
    return NPCID;
}

void APTNPCCharacter::ShowInteractionPrompt(APlayerController* PlayerController)
{
    if (PlayerController == nullptr || InteractionPromptWidgetComponent == nullptr)
    {
        return;
    }

    InteractionPromptWidgetComponent->SetVisibility(true);

    UPTNPCInteractionPromptWidget* PromptWidget =
        Cast<UPTNPCInteractionPromptWidget>(InteractionPromptWidgetComponent->GetWidget());
    if (PromptWidget != nullptr)
    {
        PromptWidget->SetupPrompt(this);
        PromptWidget->ShowPrompt();
    }
}

void APTNPCCharacter::HideInteractionPrompt(APlayerController* PlayerController)
{
    if (PlayerController == nullptr || InteractionPromptWidgetComponent == nullptr)
    {
        return;
    }

    UPTNPCInteractionPromptWidget* PromptWidget =
        Cast<UPTNPCInteractionPromptWidget>(InteractionPromptWidgetComponent->GetWidget());
    if (PromptWidget != nullptr)
    {
        PromptWidget->HidePrompt();
    }

    InteractionPromptWidgetComponent->SetVisibility(false);
}

void APTNPCCharacter::OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* OtherPawn = Cast<APawn>(OtherActor);
    APlayerController* PC = OtherPawn ? Cast<APlayerController>(OtherPawn->GetController()) : nullptr;
    if (PC == nullptr || !PC->IsLocalController())
    {
        return;
    }

    if (APTPlayerController* PTPlayerController = Cast<APTPlayerController>(PC))
    {
        PTPlayerController->RegisterNearbyNPC(this);
    }

    ShowInteractionPrompt(PC);
    OnPlayerEnterRange.Broadcast(PC);
}

void APTNPCCharacter::OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APawn* OtherPawn = Cast<APawn>(OtherActor);
    APlayerController* PC = OtherPawn ? Cast<APlayerController>(OtherPawn->GetController()) : nullptr;
    if (PC == nullptr || !PC->IsLocalController())
    {
        return;
    }

    if (APTPlayerController* PTPlayerController = Cast<APTPlayerController>(PC))
    {
        PTPlayerController->UnregisterNearbyNPC(this);
    }

    HideInteractionPrompt(PC);
    OnPlayerExitRange.Broadcast(PC);
}
