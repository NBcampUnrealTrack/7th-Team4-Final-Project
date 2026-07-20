#include "PTLevelTriggerVolume.h"
#include "Components/BoxComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Monsters/PTMonsterCharacter.h"
#include "Core/PTGameMode.h"
#include "Core/PTGameState.h"

APTLevelTriggerVolume::APTLevelTriggerVolume()
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

void APTLevelTriggerVolume::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority())
    {
        return;
    }

    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APTLevelTriggerVolume::OnTriggerBeginOverlap);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APTLevelTriggerVolume::OnTriggerEndOverlap);

    for (APTMonsterCharacter* Monster : TargetMonsters)
    {
        if (IsValid(Monster))
        {
            Monster->OnMonsterDied.AddDynamic(this, &APTLevelTriggerVolume::OnTargetMonsterDied);
        }
    }
}

void APTLevelTriggerVolume::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

void APTLevelTriggerVolume::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

void APTLevelTriggerVolume::OnTargetMonsterDied(APTMonsterCharacter* DeadMonster)
{
    ++DeadMonsterCount;
    CheckClearCondition();
}

void APTLevelTriggerVolume::CheckClearCondition()
{
    if (bTriggered)
    {
        return;
    }

    if (DeadMonsterCount < TargetMonsters.Num())
    {
        return;
    }

    APTGameState* GS = GetWorld() ? GetWorld()->GetGameState<APTGameState>() : nullptr;
    if (GS == nullptr || PlayersInVolume.Num() < GS->GetPlayerCount())
    {
        return;
    }

    bTriggered = true;
    ExecuteLevelTransition();
}

void APTLevelTriggerVolume::ExecuteLevelTransition()
{
    if (NextLevelName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PTLevelTriggerVolume] NextLevelName 미설정: %s"), *GetName());
        return;
    }

    APTGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<APTGameMode>() : nullptr;
    if (GM == nullptr)
    {
        return;
    }

    GetWorld()->GetTimerManager().SetTimer(TransitionTimerHandle, [GM, this]()
    {
        if (IsValid(GM))
        {
            GM->RequestLevelTransition(NextLevelName);
        }
    }, TransitionDelay, false);
}
