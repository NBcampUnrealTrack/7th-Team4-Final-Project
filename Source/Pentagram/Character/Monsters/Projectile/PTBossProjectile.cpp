#include "Character/Monsters/Projectile/PTBossProjectile.h"
#include "Character/PTBaseCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h" 
#include "NiagaraComponent.h"

APTBossProjectile::APTBossProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
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

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APTBossProjectile::Launch(const FVector& Direction, float InDamage, float InSpeed, const FPTHitInfo& InHitInfo, float InHomingStrength, AActor* InHomingTarget)
{
    Damage = InDamage;
    HitInfo = InHitInfo;
    HitInfo.HitDirection = Direction.GetSafeNormal();

    HomingStrength = FMath::Clamp(InHomingStrength, 0.f, 1.f);
    HomingTarget   = InHomingTarget;

    ProjectileMovement->InitialSpeed = InSpeed;
    ProjectileMovement->MaxSpeed     = InSpeed;
    ProjectileMovement->Velocity     = Direction.GetSafeNormal() * InSpeed;

    SetActorTickEnabled(HomingStrength > 0.f && HomingTarget.IsValid());
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
    if (IsValid(CollisionComp))
    {
        CollisionComp->SetSphereRadius(CollisionRadius);
    }

    if (USoundBase* Sound = FlightSound.LoadSynchronous())
    {
        FlightAudioComp = UGameplayStatics::SpawnSoundAttached(
            Sound, CollisionComp, NAME_None,
            FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true
        );
    }
}

void APTBossProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HomingTarget.IsValid())
    {
        SetActorTickEnabled(false);
        return;
    }

    const FVector CurrentDir = ProjectileMovement->Velocity.GetSafeNormal();
    const FVector ToTarget = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    const float Alpha = FMath::Clamp(HomingStrength * DeltaTime * 3.f, 0.f, 1.f);
    const FVector NewDir = FMath::Lerp(CurrentDir, ToTarget, Alpha).GetSafeNormal();

    ProjectileMovement->Velocity = NewDir * ProjectileMovement->MaxSpeed;
}

void APTBossProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("[BossProjectile::OnHit] Auth=%d OtherActor=%s Damage=%.1f"),
        (int32)HasAuthority(),
        IsValid(OtherActor) ? *OtherActor->GetName() : TEXT("null"),
        Damage);
#endif

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
        HitInfo.HitDirection = ProjectileMovement->Velocity.GetSafeNormal();
        Target->ApplyDamageWithHit(Damage, GetOwner(), HitInfo);
    }
#if !UE_BUILD_SHIPPING
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BossProjectile::OnHit] Cast<APTBaseCharacter> 실패 — OtherActor 클래스: %s"),
            IsValid(OtherActor) ? *OtherActor->GetClass()->GetName() : TEXT("null"));
    }
#endif

    MulticastPlayImpactSound(GetActorLocation());

    Destroy();
}

void APTBossProjectile::MulticastPlayImpactSound_Implementation(FVector Location)
{
    if (IsValid(FlightAudioComp))
        FlightAudioComp->Stop();

    if (USoundBase* Sound = ImpactSound.LoadSynchronous())
        UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location);
}

void APTBossProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(FlightAudioComp))
    {
        FlightAudioComp->Stop();
        FlightAudioComp = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}
