#include "Character/Monsters/Skill/PTBossShield.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"        
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"    
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"

APTBossShield::APTBossShield()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    ShieldMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
    ShieldMeshComp->SetupAttachment(GetRootComponent());
    ShieldMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShieldMeshComp->SetCastShadow(false);

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionProfileName(TEXT("Pawn"));
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        Capsule->SetGenerateOverlapEvents(true);
    }
}

void APTBossShield::InitShield(APTBossMonsterCharacter* InOwningBoss, float InMaxHP, float InRadius, float InPushForce)
{
    OwningBoss   = InOwningBoss;
    ShieldRadius = FMath::Max(InRadius, 50.f);
    PushForce    = InPushForce;

    MaxHP        = InMaxHP;
    CurrentHP    = InMaxHP;

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCapsuleSize(ShieldRadius, ShieldRadius);
    }

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->GravityScale = 0.f;
        Movement->SetMovementMode(MOVE_None);
    }

    if (IsValid(ShieldMeshComp))
    {
        const float MeshScale = ShieldRadius / FMath::Max(ShieldMeshBaseRadius, 1.f);
        ShieldMeshComp->SetWorldScale3D(FVector(MeshScale));

        if (UMaterialInterface* Material = ShieldMaterial.LoadSynchronous())
        {
            ShieldMID = UMaterialInstanceDynamic::Create(Material, this);
            ShieldMeshComp->SetMaterial(0, ShieldMID);
        }
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (OwningBoss.IsValid())
    {
        Params.AddIgnoredActor(OwningBoss.Get());
    }

    World->OverlapMultiByObjectType(
        Overlaps, GetActorLocation(), FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        FCollisionShape::MakeSphere(ShieldRadius), Params
    );

    for (const FOverlapResult& Overlap : Overlaps)
    {
        PushActorOut(Overlap.GetActor());
    }
}

float APTBossShield::ApplyDamageWithHit(float DamageAmount, AActor* Attacker, const FPTHitInfo& HitInfo)
{
    if (OwningBoss.IsValid() && Attacker == OwningBoss.Get())
    {
        return 0.f;
    }

    return Super::ApplyDamageWithHit(DamageAmount, Attacker, HitInfo);
}

void APTBossShield::ApplyHit(const FPTHitInfo& HitInfo)
{
    ApplyHitStop(HitInfo.HitStopDuration);

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    if (UNiagaraSystem* ImpactFX = HitImpactEffect.LoadSynchronous())
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ImpactFX, GetActorLocation(), FRotator::ZeroRotator);
    }

    if (USoundBase* Sound = HitSound.LoadSynchronous())
    {
        UGameplayStatics::SpawnSoundAtLocation(World, Sound, GetActorLocation());
    }
}

void APTBossShield::OnDeath()
{
    Super::OnDeath();

    if (IsValid(ShieldMeshComp))
    {
        ShieldMeshComp->SetVisibility(false);
    }

    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        if (UNiagaraSystem* BreakFX = BreakEffect.LoadSynchronous())
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BreakFX, GetActorLocation(), FRotator::ZeroRotator);
        }

        if (USoundBase* Sound = BreakSound.LoadSynchronous())
        {
            UGameplayStatics::SpawnSoundAtLocation(World, Sound, GetActorLocation());
        }
    }

    if (!bSuppressDestroyNotify && OwningBoss.IsValid())
    {
        OwningBoss->OnShieldDestroyed();
    }

    SetLifeSpan(2.f);
}

void APTBossShield::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (!HasAuthority())
    {
        return;
    }

    PushActorOut(OtherActor);
}

void APTBossShield::PushActorOut(AActor* TargetActor)
{
    if (!IsValid(TargetActor) || TargetActor == this)
    {
        return;
    }

    if (OwningBoss.IsValid() && TargetActor == OwningBoss.Get())
    {
        return;
    }

    APTBaseCharacter* TargetChar = Cast<APTBaseCharacter>(TargetActor);
    if (!IsValid(TargetChar) || TargetChar->IsDead())
    {
        return;
    }

    ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
    if (!IsValid(TargetCharacter))
    {
        return;
    }

    const FVector BossLocation = OwningBoss.IsValid()
        ? OwningBoss->GetActorLocation()
        : GetActorLocation();

    FVector Direction = TargetActor->GetActorLocation() - BossLocation;
    Direction.Z = 0.f;
    Direction = Direction.GetSafeNormal();
    if (Direction.IsNearlyZero())
    {
        Direction = TargetActor->GetActorForwardVector();
    }

    FVector SafeLocation = BossLocation + Direction * (ShieldRadius + 100.f);
    SafeLocation.Z = TargetActor->GetActorLocation().Z;
    TargetActor->SetActorLocation(SafeLocation, true);

    TargetCharacter->LaunchCharacter(Direction * PushForce, true, false);
}
