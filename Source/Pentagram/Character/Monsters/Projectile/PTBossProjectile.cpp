#include "Character/Monsters/Projectile/PTBossProjectile.h"
#include "Character/PTBaseCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"

APTBossProjectile::APTBossProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->InitSphereRadius(20.f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionComp->OnComponentHit.AddDynamic(this, &APTBossProjectile::OnHit);
    RootComponent = CollisionComp;

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale   = 0.f;

    NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
    NiagaraComp->SetupAttachment(RootComponent);
}

void APTBossProjectile::Launch(const FVector& Direction, float InDamage, float InSpeed, const FPTHitInfo& InHitInfo)
{
    Damage = InDamage;
    HitInfo = InHitInfo;

    HitInfo.HitDirection = Direction.GetSafeNormal();

    ProjectileMovement->InitialSpeed = InSpeed;
    ProjectileMovement->MaxSpeed     = InSpeed;
    ProjectileMovement->Velocity     = Direction.GetSafeNormal() * InSpeed;
}

void APTBossProjectile::IgnoreActor(AActor* ActorToIgnore)
{
    if (!IsValid(ActorToIgnore) || !IsValid(CollisionComp))
    {
        return;
    }

    CollisionComp->IgnoreActorWhenMoving(ActorToIgnore, true);
}

void APTBossProjectile::BeginPlay()
{
	Super::BeginPlay();
    SetLifeSpan(LifeSpan);
}

void APTBossProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!IsValid(OtherActor))
    {
        return;
    }

    if (OtherActor == GetOwner() || OtherActor == GetInstigator())
    {
        return;
    }

    if (APTBaseCharacter* Target = Cast<APTBaseCharacter>(OtherActor))
    {
        Target->ApplyDamageWithHit(Damage, GetOwner(), HitInfo);
    }

    Destroy();
}


