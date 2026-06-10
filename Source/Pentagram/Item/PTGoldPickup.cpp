#include "Item/PTGoldPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Core/Subsystems/PTEconomySubsystem.h"

APTGoldPickup::APTGoldPickup()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    GoldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoldMesh"));
    RootComponent = GoldMesh;
    GoldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(100.f);
    CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void APTGoldPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
    if (!HasAuthority() || bPickedUp)
    {
        return;
    }

    APTPlayerCharacter* Player = Cast<APTPlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    APTBasePlayerState* PS = Player->GetPlayerState<APTBasePlayerState>();
    if (!PS)
    {
        return;
    }

    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        return;
    }

    UPTEconomySubsystem* EconomySys = GI->GetSubsystem<UPTEconomySubsystem>();
    if (!EconomySys)
    {
        return;
    }

    bPickedUp = true;
    UE_LOG(LogTemp, Warning, TEXT("[PTGoldPickup] 골드 획득 — 금액: %d / 플레이어: %s"),
        GoldAmount, *PS->GetPlayerName());
    EconomySys->AddGold(PS, GoldAmount);
    Destroy();
}
