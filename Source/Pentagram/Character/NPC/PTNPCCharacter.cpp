#include "Character/NPC/PTNPCCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/Subsystems/PTQuestSubsystem.h"
#include "GameFramework/Pawn.h"
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

void APTNPCCharacter::Interact_Implementation(AActor* InteractorCharacter)
{
    if (!HasAuthority() || !InteractorCharacter)
    {
        return;
    }

    APawn* InteractPawn = Cast<APawn>(InteractorCharacter);
    APlayerController* InteractPlayerController = InteractPawn ? Cast<APlayerController>(InteractPawn->GetController()) : nullptr;

    if (InteractPlayerController == nullptr)
    {
        return;
    }

    // 해당 플레이어의 퀘스트 진척도 업데이트
    if (!NPCID.IsNone())
    {
        UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
        if (QuestSubsystem)
        {
            QuestSubsystem->UpdateQuestProgress(EPTQuestConditionType::TalkToNPC, NPCID);
        }
    }

    // 말을 건 플레이어만 대화창 브로드캐스트
    OnDialogueStarted.Broadcast(InteractPlayerController);
}

// 퀘스트 수락
void APTNPCCharacter::ServerAcceptQuest_Implementation(FName QuestID)
{
    if (QuestID.IsNone() || !QuestIDs.Contains(QuestID))
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem)
    {
        QuestSubsystem->AcceptQuest(QuestID);
    }
}

// 퀘스트 보상 
void APTNPCCharacter::ServerRewardQuest_Implementation(FName QuestID)
{
    if (QuestID.IsNone() || !QuestIDs.Contains(QuestID))
    {
        return;
    }

    UPTQuestSubsystem* QuestSubsystem = GetGameInstance()->GetSubsystem<UPTQuestSubsystem>();
    if (QuestSubsystem)
    {
        QuestSubsystem->RewardQuest(QuestID);
    }
}

FName APTNPCCharacter::GetNPCID() const { return NPCID; }
const TArray<FName>& APTNPCCharacter::GetQuestIDs() const { return QuestIDs; }

// 안전을 위해 범위 진입/이탈 체크도 오직 '서버'에서만 판단하여 해당 유저에게 이벤트를 전송.
void APTNPCCharacter::OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    APlayerController* PC = OtherActor ? OtherActor->GetInstigatorController<APlayerController>() : nullptr;
    if (PC)
    {
        OnPlayerEnterRange.Broadcast(PC);
    }
}

void APTNPCCharacter::OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    APlayerController* PC = OtherActor ? OtherActor->GetInstigatorController<APlayerController>() : nullptr;
    if (PC)
    {
        OnPlayerExitRange.Broadcast(PC);
    }
}
