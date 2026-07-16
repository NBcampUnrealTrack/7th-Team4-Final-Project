// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionActor/PTInteractionActor.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/Player/PTPlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "UI/Widget/Item/PTDropItemNameWidget.h"
#include "UI/Widget/NPC/PTNPCInteractionPromptWidget.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
APTInteractionActor::APTInteractionActor()
{
	PrimaryActorTick.bCanEverTick = false; 

    ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ActorMesh"));
    RootComponent = ActorMesh;

    InteractionRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRangeSphere"));
    InteractionRangeSphere->SetupAttachment(RootComponent);
    InteractionRangeSphere->SetSphereRadius(InteractionRadius);
    InteractionRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionRangeSphere->SetCollisionObjectType(ECC_WorldDynamic);
    InteractionRangeSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionRangeSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
    InteractionRangeSphere->SetGenerateOverlapEvents(true);

    RespawnSavedWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("RespawnSavedWidgetComponent"));
    RespawnSavedWidgetComponent->SetupAttachment(RootComponent);
    RespawnSavedWidgetComponent->SetRelativeLocation(RespawnSavedWidgetRelativeLocation);
    RespawnSavedWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    RespawnSavedWidgetComponent->SetDrawAtDesiredSize(true);
    RespawnSavedWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
    RespawnSavedWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RespawnSavedWidgetComponent->SetGenerateOverlapEvents(false);
    RespawnSavedWidgetComponent->SetWidgetClass(UPTDropItemNameWidget::StaticClass());
    RespawnSavedWidgetComponent->SetVisibility(false);

    static ConstructorHelpers::FClassFinder<UPTNPCInteractionPromptWidget> PromptWidgetFinder(
        TEXT("/Game/Pentagram/UI/Widget/NPC/WBP_PTNPCInteractionPromptWidget"));
    if (PromptWidgetFinder.Succeeded())
    {
        InteractionPromptWidgetClass = PromptWidgetFinder.Class;
    }

    InteractionPromptWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidgetComponent"));
    InteractionPromptWidgetComponent->SetupAttachment(RootComponent);
    InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
    InteractionPromptWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    InteractionPromptWidgetComponent->SetDrawAtDesiredSize(true);
    InteractionPromptWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
    InteractionPromptWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractionPromptWidgetComponent->SetGenerateOverlapEvents(false);
    if (InteractionPromptWidgetClass != nullptr)
    {
        InteractionPromptWidgetComponent->SetWidgetClass(InteractionPromptWidgetClass);
    }
    InteractionPromptWidgetComponent->SetVisibility(false);

    bReplicates = true; // [멀티플레이어 환경] 서버가 이 액터의 상태를 동기화하도록 설정 
    InteractionType = EInteractType::None;
    bIsInteracted = false; // 초기화 
} 


void APTInteractionActor::BeginPlay()
{
    Super::BeginPlay();

    InteractionRangeSphere->SetSphereRadius(InteractionRadius);
    InteractionRangeSphere->OnComponentBeginOverlap.AddUniqueDynamic(
        this,
        &APTInteractionActor::OnInteractionRangeBeginOverlap);
    InteractionRangeSphere->OnComponentEndOverlap.AddUniqueDynamic(
        this,
        &APTInteractionActor::OnInteractionRangeEndOverlap);

    RespawnSavedWidgetComponent->SetRelativeLocation(RespawnSavedWidgetRelativeLocation);
    RespawnSavedWidgetComponent->SetVisibility(false);

    InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
    if (InteractionPromptWidgetClass != nullptr)
    {
        InteractionPromptWidgetComponent->SetWidgetClass(InteractionPromptWidgetClass);
    }
    InteractionPromptWidgetComponent->SetVisibility(false);
}

void APTInteractionActor::OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    APawn* OtherPawn = Cast<APawn>(OtherActor);
    APTPlayerController* PlayerController = OtherPawn
        ? Cast<APTPlayerController>(OtherPawn->GetController())
        : nullptr;
    if (PlayerController == nullptr || !PlayerController->IsLocalController())
    {
        return;
    }

    PlayerController->RegisterNearbyInteractionActor(this);
    HideRespawnSavedLabel();
    ShowInteractionPrompt();
}

void APTInteractionActor::OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APawn* OtherPawn = Cast<APawn>(OtherActor);
    APTPlayerController* PlayerController = OtherPawn
        ? Cast<APTPlayerController>(OtherPawn->GetController())
        : nullptr;
    if (PlayerController == nullptr || !PlayerController->IsLocalController())
    {
        return;
    }

    PlayerController->UnregisterNearbyInteractionActor(this);
    HideInteractionPrompt();
    HideRespawnSavedLabel();
}

void APTInteractionActor::Interact_Implementation(AActor* InteractorCharacter)
{
    // 보안, 무결성을 위해 오직 '서버' 권한이 있는 곳에서만 데이터를 수정.
    if (!HasAuthority() || !InteractorCharacter) return;

    APTPlayerCharacter* PlayerChar = Cast<APTPlayerCharacter>(InteractorCharacter);
    if (!PlayerChar) return;

    switch (InteractionType)
    {
    case EInteractType::Box:
        // 이미 열린 상자라면 중복 처리를 차단 (아이템 무한 복사 버그 방지)
        if (bIsInteracted) return;
        bIsInteracted = true; // 서버 장부 잠금
        // [상자 로직] 서버에서 안전한 아이템 지급 처리 등
        UE_LOG(LogTemp, Log, TEXT("보물상자 상호작용 - %s 아이템 지급"), *RewardItemID.ToString());
        Multicast_PlayInteractionEffects(); // 상자가 열리는 연출은 방에 있는 모두에게 똑같이 보여줌.
        break;

    case EInteractType::Door:
        if (bIsInteracted) return;
        bIsInteracted = true;

        UE_LOG(LogTemp, Log, TEXT("문 상호작용 활성화"));
        Multicast_PlayInteractionEffects();
        break;

    case EInteractType::Lever:
        if (bIsInteracted) return;
        bIsInteracted = true;

        UE_LOG(LogTemp, Log, TEXT("레버 상호작용 활성화"));
        Multicast_PlayInteractionEffects();
        break;

    case EInteractType::RespawnPoint:
        // [리스폰 등록 로직] 여기서 통합 처리
        // 리스폰 지점은 개인화 처리
        if (APTBasePlayerState* PS = PlayerChar->GetPlayerState<APTBasePlayerState>())
        {
            FVector RespawnLocation = PlayerChar->GetActorLocation();
            PS->SetSavedRespawnLocation(RespawnLocation);
            UE_LOG(LogTemp, Log, TEXT("리스폰 지점 등록 완료: %s"), *RespawnLocation.ToString());

            // 연출 
            SetOwner(PlayerChar);
            Client_PlayLocalEffects();
        }
        break;

    default:
        break;
    }
}

// 상호작용한 클라이언트 본인에게만 연출 효과
void APTInteractionActor::Client_PlayLocalEffects_Implementation()
{
    if (InteractionType == EInteractType::RespawnPoint)
    {
        ShowRespawnSavedLabel();
    }

    OnPlayInteractionEffects();
}
// [멀티플레이어 동기화] 상호작용 시 모든 클라이언트에게 연출 효과
void APTInteractionActor::Multicast_PlayInteractionEffects_Implementation()
{
    OnPlayInteractionEffects();
}

void APTInteractionActor::ShowInteractionPrompt()
{
    if (InteractionPromptWidgetComponent == nullptr)
    {
        return;
    }

    InteractionPromptWidgetComponent->SetVisibility(true);
    InteractionPromptWidgetComponent->InitWidget();

    UPTNPCInteractionPromptWidget* PromptWidget =
        Cast<UPTNPCInteractionPromptWidget>(InteractionPromptWidgetComponent->GetUserWidgetObject());
    if (PromptWidget != nullptr)
    {
        PromptWidget->ShowPrompt();
    }
}

void APTInteractionActor::HideInteractionPrompt()
{
    if (InteractionPromptWidgetComponent == nullptr)
    {
        return;
    }

    UPTNPCInteractionPromptWidget* PromptWidget =
        Cast<UPTNPCInteractionPromptWidget>(InteractionPromptWidgetComponent->GetUserWidgetObject());
    if (PromptWidget != nullptr)
    {
        PromptWidget->HidePrompt();
    }

    InteractionPromptWidgetComponent->SetVisibility(false);
}

void APTInteractionActor::ShowRespawnSavedLabel()
{
    if (RespawnSavedWidgetComponent == nullptr)
    {
        return;
    }

    HideInteractionPrompt();
    RespawnSavedWidgetComponent->SetVisibility(true);
    RespawnSavedWidgetComponent->InitWidget();

    UPTDropItemNameWidget* SavedLabelWidget =
        Cast<UPTDropItemNameWidget>(RespawnSavedWidgetComponent->GetUserWidgetObject());
    if (SavedLabelWidget != nullptr)
    {
        SavedLabelWidget->SetItemName(
            NSLOCTEXT("PTInteraction", "RespawnPointSaved", "Respawn Point Saved"));
    }
}

void APTInteractionActor::HideRespawnSavedLabel()
{
    if (RespawnSavedWidgetComponent != nullptr)
    {
        RespawnSavedWidgetComponent->SetVisibility(false);
    }
}

