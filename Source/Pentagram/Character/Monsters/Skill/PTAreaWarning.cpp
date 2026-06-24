#include "Character/Monsters/Skill/PTAreaWarning.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

APTAreaWarning::APTAreaWarning()
{
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    WarningMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WarningMesh"));
    WarningMesh->SetupAttachment(SceneRoot);
    WarningMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WarningMesh->SetVisibility(false);

    BorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BorderMesh"));
    BorderMesh->SetupAttachment(SceneRoot);
    BorderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BorderMesh->SetVisibility(false);
    BorderMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.1f));
}

void APTAreaWarning::Launch(const FVector& GroundLocation, float InFallDuration, UNiagaraSystem* InFallEffect, UNiagaraSystem* InImpactEffect, float StartHeight, float InMaxRadius)
{
    TargetLocation      = GroundLocation;
    StartLocation       = GroundLocation + FVector(0.f, 0.f, StartHeight);
    FallDuration        = FMath::Max(InFallDuration, 0.01f);
    ElapsedTime         = 0.f;
    bLaunched           = true;
    bLanded             = false;
    PendingImpactEffect = InImpactEffect;
    MaxRadius           = InMaxRadius;
    CachedStartHeight   = StartHeight;

    SetActorLocation(GroundLocation);

    if (InFallEffect)
    {
        FallEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            InFallEffect, GetRootComponent(), NAME_None,
            FVector(0.f, 0.f, StartHeight), FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset, true);
    }

    if (IsValid(WarningMesh) && WarningMesh->GetStaticMesh())
    {
        WarningMesh->SetRelativeScale3D(FVector::ZeroVector);
        WarningMesh->SetVisibility(true);
    }

    if (IsValid(BorderMesh) && BorderMesh->GetStaticMesh())
    {
        const float BorderScale = MaxRadius / 100.f;
        BorderMesh->SetRelativeScale3D(FVector(BorderScale, BorderScale, 1.f));
        BorderMesh->SetVisibility(true);
    }

    SetActorTickEnabled(true);
}

void APTAreaWarning::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!bLaunched || bLanded)
    {
        return;
    }

    ElapsedTime += DeltaTime;
    const float Alpha = FMath::Clamp(ElapsedTime / FallDuration, 0.f, 1.f);
    SetActorLocation(FMath::Lerp(StartLocation, TargetLocation, Alpha));

    if (IsValid(FallEffectComp))
    {
        const float CurrentZ = FMath::Lerp(CachedStartHeight, 0.f, Alpha);
        FallEffectComp->SetRelativeLocation(FVector(0.f, 0.f, CurrentZ));
    }

    if (IsValid(WarningMesh) && WarningMesh->GetStaticMesh())
    {
        const float MeshScale = FMath::Lerp(0.f, MaxRadius / 100.f, Alpha);
        WarningMesh->SetRelativeScale3D(FVector(MeshScale, MeshScale, 1.f));
    }

    if (Alpha >= 1.f)
    {
        OnLanded();
    }
}

void APTAreaWarning::OnLanded()
{
    bLanded = true;
    SetActorTickEnabled(false);

    if (IsValid(FallEffectComp))
    {
        FallEffectComp->Deactivate();
    }

    if (IsValid(WarningMesh))
    {
        WarningMesh->SetVisibility(false);
    }

    if (IsValid(BorderMesh))
    {
        BorderMesh->SetVisibility(false);
    }

    if (PendingImpactEffect)
    {
        UNiagaraComponent* ImpactComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), PendingImpactEffect, TargetLocation,
            FRotator::ZeroRotator, FVector::OneVector, true, false);

        if (IsValid(ImpactComp))
        {
            ImpactComp->SetNiagaraVariableFloat(FString("User.Radius"), MaxRadius);
            ImpactComp->Activate(true);
        }
    }

    SetLifeSpan(3.f);
}
