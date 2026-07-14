#include "PTSkillIndicatorActor.h"
#include "Components/DecalComponent.h"

APTSkillIndicatorActor::APTSkillIndicatorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
    Decal->SetupAttachment(Root);
    // 데칼은 로컬 -X축으로 투영 → 지면에 쏘려면 액터를 아래로 눕힘
    Decal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    Decal->SetVisibility(false);

    RangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecal"));
    RangeDecal->SetupAttachment(Root);
    RangeDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    RangeDecal->SetVisibility(false);
}

void APTSkillIndicatorActor::ShowLine(const FVector& Origin, const FVector& Dir, float Range, float Width)
{
    if (LineMID) Decal->SetDecalMaterial(LineMID);
    const FRotator Yaw = Dir.Rotation();
    Decal->SetWorldLocation(Origin + Dir * (Range * 0.5f));
    Decal->SetWorldRotation(FRotator(-90.f, Yaw.Yaw, 0.f));
    Decal->DecalSize = FVector(256.f, Width * 0.5f, Range * 0.5f);
    Decal->SetVisibility(true);
}

void APTSkillIndicatorActor::ShowCircle(const FVector& Center, float Radius)
{
    if (CircleMID) Decal->SetDecalMaterial(CircleMID);
    Decal->SetWorldLocation(Center);
    Decal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
    Decal->DecalSize = FVector(256.f, Radius, Radius);
    Decal->SetVisibility(true);
}

void APTSkillIndicatorActor::ShowCone(const FVector& Origin, const FVector& Dir, float Range, float AngleDeg)
{
    if (ConeMID)
    {
        Decal->SetDecalMaterial(ConeMID);
        ConeMID->SetScalarParameterValue(TEXT("HalfAngle"), FMath::DegreesToRadians(AngleDeg * 0.5f));
    }

    const FRotator Yaw = Dir.Rotation();

    Decal->SetWorldLocation(Origin);
    Decal->SetWorldRotation(FRotator(-90.f, Yaw.Yaw, 0.f));
    Decal->DecalSize = FVector(256.f, Range, Range);
    Decal->SetVisibility(true);
}

void APTSkillIndicatorActor::ShowSelfCircle(const FVector& Center, float Radius)
{
    ShowCircle(Center, Radius);
}

void APTSkillIndicatorActor::HideIndicator()
{
    Decal->SetVisibility(false);
    RangeDecal->SetVisibility(false);
}

void APTSkillIndicatorActor::ShowRange(const FVector& Center, float CastRange)
{
    if (RangeMaterial) RangeDecal->SetDecalMaterial(RangeMaterial); // RangeDecal은 월드에 고정하고 싶으면 컴포넌트 위치를 캐릭터로

    RangeDecal->SetWorldLocation(Center);
    RangeDecal->DecalSize = FVector(256.f, CastRange, CastRange);
    RangeDecal->SetVisibility(true);
}

void APTSkillIndicatorActor::HideRange()
{
    RangeDecal->SetVisibility(false);
}

void APTSkillIndicatorActor::BeginPlay()
{
    Super::BeginPlay();

    if (LineMaterial)   LineMID   = UMaterialInstanceDynamic::Create(LineMaterial,   this);
    if (CircleMaterial) CircleMID = UMaterialInstanceDynamic::Create(CircleMaterial, this);
    if (ConeMaterial)   ConeMID   = UMaterialInstanceDynamic::Create(ConeMaterial,   this);
}

