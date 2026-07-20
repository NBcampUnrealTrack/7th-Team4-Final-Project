

#include "InteractionActor/PTLevelTrigger.h"
#include "Components/BoxComponent.h"
#include "Core/Subsystems/PTSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"


APTLevelTrigger::APTLevelTrigger()
{
    // 루트 컴포넌트로 박스 콜리전 생성 및 설정
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;

    // 오버랩 이벤트 바인딩 
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APTLevelTrigger::OnOverlapBegin);
}

void APTLevelTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || bTravelRequested || TargetMapName.IsEmpty())
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character == nullptr)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (PC == nullptr)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    bTravelRequested = true;

    UGameInstance* GameInstance = World->GetGameInstance();
    UPTSaveSubsystem* SaveSubsystem =
        GameInstance != nullptr ? GameInstance->GetSubsystem<UPTSaveSubsystem>() : nullptr;
    if (SaveSubsystem == nullptr || !SaveSubsystem->SaveAllAuthorityPlayers(false))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Save] Zone travel could not save an authority player."));
    }

    FString TravelURL = TargetMapName;
    if (!TargetActorTag.IsNone())
    {
        TravelURL += FString::Printf(TEXT("?PlayerActorTag=%s"), *TargetActorTag.ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("[Save] Zone travel routing via ServerTravel. Target=%s"), *TravelURL);

    World->ServerTravel(TravelURL, false);
}

void APTLevelTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
