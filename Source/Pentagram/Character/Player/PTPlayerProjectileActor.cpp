#include "PTPlayerProjectileActor.h"

#include "NiagaraComponent.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"

APTPlayerProjectileActor::APTPlayerProjectileActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->SetupAttachment(Root);
    CollisionComp->InitSphereRadius(50.f);
    CollisionComp->SetCollisionProfileName(TEXT("OverlapAll"));
    CollisionComp->SetGenerateOverlapEvents(true);
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &APTPlayerProjectileActor::OnProjectileOverlap);

    TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComponent"));
    TrailComponent->SetupAttachment(Root);
    TrailComponent->bAutoActivate = false;
}

void APTPlayerProjectileActor::InitProjectile(
    APTPlayerCharacter* InAttacker,
    const FVector& InDirection,
    float InSpeed,
    float InHitRadius,
    float InMaxRange,
    float InDamageMultiplier,
    bool bInPenetrate,
    bool bInApplyDamage,
    UNiagaraSystem* InTrailEffect,
    UNiagaraSystem* InHitEffect,
    USoundBase* InHitSound,
    float InKnockbackForce,
    float InKnockbackZForce,
    float InHitStopDuration,
    float InStaggerDuration,
    EHitReactionType InHitReactionType,
    bool bInDrawDebug)
{
    Attacker         = InAttacker;
    Direction        = InDirection.GetSafeNormal();
    Speed            = InSpeed;
    HitRadius        = InHitRadius;
    MaxRange         = InMaxRange;
    DamageMultiplier = InDamageMultiplier;
    bPenetrate       = bInPenetrate;
    bApplyDamage     = bInApplyDamage;
    HitSound         = InHitSound;
    HitEffect        = InHitEffect;
    KnockbackForce   = InKnockbackForce;
    KnockbackZForce  = InKnockbackZForce;
    HitStopDuration  = InHitStopDuration;
    StaggerDuration  = InStaggerDuration;
    HitRadius        = InHitRadius;
    HitReactionType  = InHitReactionType;
    bDrawDebug       = bInDrawDebug;

    if (CollisionComp)
    {
        CollisionComp->SetSphereRadius(HitRadius);
    }

    StartLocation = GetActorLocation();
    SetActorRotation(Direction.Rotation());

    if (InTrailEffect)
    {
        TrailComponent->SetAsset(InTrailEffect);
        TrailComponent->Activate(true);
    }

    const float SafetyLifeSpan = (Speed > 0.f) ? (MaxRange / Speed) + 1.0f : 5.0f;
    SetLifeSpan(SafetyLifeSpan);
}

void APTPlayerProjectileActor::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bExpired) return;
    if (!OtherActor || OtherActor == this || OtherActor == Attacker.Get()) return;

    APTBaseCharacter* Target = Cast<APTBaseCharacter>(OtherActor);
    const bool bIsEnemyCharacter = Target && !Cast<APTPlayerCharacter>(Target);

    if (bIsEnemyCharacter && bPenetrate)
    {
        return;
    }

    const FVector ImpactPoint = SweepResult.ImpactPoint;
    PlayHitEffects(ImpactPoint.IsZero() ? GetActorLocation() : ImpactPoint);

    ExpireProjectile();
}

void APTPlayerProjectileActor::BeginPlay()
{
    Super::BeginPlay();
}

void APTPlayerProjectileActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bExpired) return;

    const FVector NewLocation = GetActorLocation() + Direction * Speed * DeltaTime;
    SetActorLocation(NewLocation);

    if (FVector::Dist(StartLocation, NewLocation) > MaxRange)
    {
        ExpireProjectile();
        return;
    }

#if WITH_EDITOR
    if (bDrawDebug)
    {
        DrawDebugSphere(GetWorld(), NewLocation, HitRadius, 8, FColor::Cyan, false, 0.1f);
    }
#endif

    // 데미지 판정은 서버에서만
    if (!bApplyDamage) return;

    APTPlayerCharacter* AttackerPtr = Attacker.Get();
    if (!AttackerPtr) return;

    TArray<AActor*> OverlapActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        NewLocation,
        HitRadius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ AttackerPtr, this },
        OverlapActors
    );

    for (AActor* HitActor : OverlapActors)
    {
        if (HitActors.Contains(TWeakObjectPtr<AActor>(HitActor))) continue;

        APTBaseCharacter* Target = Cast<APTBaseCharacter>(HitActor);
        if (!Target || Cast<APTPlayerCharacter>(Target)) continue;

        HitActors.Add(TWeakObjectPtr<AActor>(HitActor));

        const float FinalDamage = AttackerPtr->GetTotalAttack() * DamageMultiplier;

        FPTHitInfo HitInfo;
        HitInfo.Attacker        = AttackerPtr;
        HitInfo.HitDirection    = Direction;
        HitInfo.KnockbackForce  = KnockbackForce;
        HitInfo.KnockbackZForce = KnockbackZForce;
        HitInfo.HitStopDuration = HitStopDuration;
        HitInfo.StaggerDuration = StaggerDuration;
        HitInfo.HitReactionType = HitReactionType;

        Target->ApplyDamageWithHit(FinalDamage, AttackerPtr, HitInfo);

        UE_LOG(LogTemp, Log, TEXT("Projectile %s 명중 / 데미지 %.1f"),
            *Target->GetName(), FinalDamage);

        if (!bPenetrate)
        {
            ExpireProjectile();
            break;
        }

        if (HitSound && AttackerPtr->SkillComp)
        {
            AttackerPtr->SkillComp->Multicast_PlayHitSound(HitSound, Target->GetActorLocation());
        }

        if (!bPenetrate)
        {
            ExpireProjectile();
            break;
        }
    }
}

void APTPlayerProjectileActor::PlayHitEffects(const FVector& Location)
{
    if (HitEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Location, Direction.Rotation());
    }

    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Location);
    }
}

void APTPlayerProjectileActor::ExpireProjectile()
{
    if (bExpired) return;
    bExpired = true;

    SetActorTickEnabled(false);
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);


    if (TrailComponent)
    {
        TrailComponent->DeactivateImmediate();
        TrailComponent->SetVisibility(false, true);
    }

    Destroy();
}
