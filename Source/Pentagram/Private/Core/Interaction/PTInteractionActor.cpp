// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Interaction/PTInteractionActor.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Player/PTBasePlayerState.h"


// Sets default values
APTInteractionActor::APTInteractionActor()
{
	PrimaryActorTick.bCanEverTick = false; 

    InteractionActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractionActorMesh"));
    RootComponent = InteractionActorMesh;

    bReplicates = true; // [멀티플레이어 환경] 서버가 이 액터의 상태를 동기화하도록 설정 
    InteractionType = EInteractionType::None;
    bIsInteracted = false; // 초기화 
} 


void APTInteractionActor::Interact_Implementation(AActor* InteractorCharacter)
{
    // 보안, 무결성을 위해 오직 '서버' 권한이 있는 곳에서만 데이터를 수정.
    if (!HasAuthority() || !InteractorCharacter) return;

    APTPlayerCharacter* PlayerChar = Cast<APTPlayerCharacter>(InteractorCharacter);
    if (!PlayerChar) return;

    switch (InteractionType)
    {
    case EInteractionType::TreasureBox:
        // 이미 열린 상자라면 중복 처리를 차단 (아이템 무한 복사 버그 방지)
        if (bIsInteracted) return;
        bIsInteracted = true; // 서버 장부 잠금
        // [상자 로직] 서버에서 안전한 아이템 지급 처리 등
        UE_LOG(LogTemp, Log, TEXT("보물상자 상호작용 - %s 아이템 지급"), *RewardItemID.ToString());
        Multicast_PlayInteractionEffects(); // 상자가 열리는 연출은 방에 있는 모두에게 똑같이 보여줌.
        break;

    case EInteractionType::Door:
        if (bIsInteracted) return;
        bIsInteracted = true;

        UE_LOG(LogTemp, Log, TEXT("문 상호작용 활성화"));
        Multicast_PlayInteractionEffects();
        break;

    case EInteractionType::Lever:
        if (bIsInteracted) return;
        bIsInteracted = true;

        UE_LOG(LogTemp, Log, TEXT("레버 상호작용 활성화"));
        Multicast_PlayInteractionEffects();
        break;

    case EInteractionType::RespawnPoint:
        // [리스폰 등록 로직] 여기서 통합 처리
        // 리스폰 지점은 개인화 처리
        if (APTBasePlayerState* PS = PlayerChar->GetPlayerState<APTBasePlayerState>())
        {
            FVector RespawnLocation = GetActorLocation();
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

// 상호작용한 클라이언트 본인의 화면에서만 연출 효과 
void APTInteractionActor::Client_PlayLocalEffects_Implementation()
{
    OnPlayInteractionEffects();
}
// [멀티플레이어 동기화] 모든 클라이언트에게 상호작용 연출 효과 
void APTInteractionActor::Multicast_PlayInteractionEffects_Implementation()
{
    OnPlayInteractionEffects();
}

