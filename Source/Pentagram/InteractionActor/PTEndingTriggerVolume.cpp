#include "PTEndingTriggerVolume.h"
#include "Components/BoxComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Core/PTGameState.h"

APTEndingTriggerVolume::APTEndingTriggerVolume()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;    // 서버 전용 로직, 복제 불필요

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(300.f, 300.f, 200.f));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APTEndingTriggerVolume::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority())
    {
        return;
    }

    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APTEndingTriggerVolume::OnTriggerBeginOverlap);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APTEndingTriggerVolume::OnTriggerEndOverlap);
}

void APTEndingTriggerVolume::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || bTriggered)
    {
        return;
    }

    if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(OtherActor))
    {
        PlayersInVolume.Add(Player);
        CheckClearCondition();
    }
}

void APTEndingTriggerVolume::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    if (APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(OtherActor))
    {
        PlayersInVolume.Remove(Player);
    }
}

void APTEndingTriggerVolume::CheckClearCondition()
{
    if (bTriggered)
    {
        return;
    }

    APTGameState* GS = GetWorld() ? GetWorld()->GetGameState<APTGameState>() : nullptr;
    if (GS == nullptr || PlayersInVolume.Num() < GS->GetPlayerCount())
    {
        return;
    }

    bTriggered = true;
    GS->Multicast_NotifyEndingTriggered();
}
