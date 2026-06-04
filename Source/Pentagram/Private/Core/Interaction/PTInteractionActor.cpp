// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Interaction/PTInteractionActor.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Player/PTBasePlayerState.h"


// Sets default values
APTInteractionActor::APTInteractionActor()
{
	PrimaryActorTick.bCanEverTick = true; 

    InteractionActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractionActorMesh"));
    RootComponent = InteractionActorMesh;

    bReplicates = true; // [멀티플레이어 환경] 서버가 이 액터의 상태를 동기화하도록 설정 

    InteractionType = EInteractionType::None; 
} 


void APTInteractionActor::Interact_Implementation(AActor* InteractorCharacter)
{
    if (!InteractorCharacter) return;

    APTPlayerCharacter* PlayerChar = Cast<APTPlayerCharacter>(InteractorCharacter);
    if (!PlayerChar) return;

    switch (InteractionType) 
    {
    case EInteractionType::TreasureBox:
        // [상자 로직] 아이템 지급 처리 등
        UE_LOG(LogTemp, Log, TEXT("[C++] 보물상자 상호작용 - %s 아이템 지급"), *RewardItemID.ToString());
        break;

    case EInteractionType::Door:
        // [문 로직] 필요하다면 C++ 단 장부 처리
        UE_LOG(LogTemp, Log, TEXT("[C++] 문 상호작용 활성화"));
        break;

    case EInteractionType::RespawnPoint:
        // [리스폰 등록 로직] 여기서 통합 처리 
        if (APTBasePlayerState* PS = PlayerChar->GetPlayerState<APTBasePlayerState>())
        {
            FVector RespawnLocation = GetActorLocation(); 
            PS->SetSavedRespawnLocation(RespawnLocation); 
            UE_LOG(LogTemp, Log, TEXT("[C++] 리스폰 지점 등록 완료: %s"), *RespawnLocation.ToString());
        }
        break;

    case EInteractionType::Lever:
        UE_LOG(LogTemp, Log, TEXT("[C++] 레버 상호작용 활성화"));
        break;

    default:
        break;
    }

    // 데이터 처리가 끝나면 블루프린트 연출(파티클, 애니메이션)이벤트 실행
    OnPlayInteractionEffects();
}

