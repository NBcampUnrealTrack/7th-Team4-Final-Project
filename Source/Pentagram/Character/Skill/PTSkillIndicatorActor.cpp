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
}

void APTSkillIndicatorActor::ShowLine(const FVector& Origin, const FVector& Dir, float Range, float Width)
{
    if (LineMID) Decal->SetDecalMaterial(LineMID);
    const FRotator Yaw = Dir.Rotation();
    SetActorLocation(Origin + Dir * (Range * 0.5f));
    SetActorRotation(FRotator(0.f, Yaw.Yaw, 0.f));
    // DecalSize = ( Y=폭 반값, Z=길이 반값). 데칼 축 방향은 에디터에서 미세조정
    Decal->DecalSize = FVector(256.f, Width * 0.5f, Range * 0.5f);
    Decal->SetVisibility(true);
}

void APTSkillIndicatorActor::ShowCircle(const FVector& Center, float Radius)
{
    if (CircleMID) Decal->SetDecalMaterial(CircleMID);
    SetActorLocation(Center);
    SetActorRotation(FRotator::ZeroRotator);
    Decal->DecalSize = FVector(256.f, Radius, Radius);
    Decal->SetVisibility(true);
}

void APTSkillIndicatorActor::ShowCone(const FVector& Origin, const FVector& Dir, float Range, float AngleDeg)
{
    if (ConeMID)
    {
        Decal->SetDecalMaterial(ConeMID);
        ConeMID->SetScalarParameterValue(TEXT("HalfAngle"),
            FMath::DegreesToRadians(AngleDeg * 0.5f));
    }
    const FRotator Yaw = Dir.Rotation();
    SetActorLocation(Origin);
    SetActorRotation(FRotator(0.f, Yaw.Yaw, 0.f));
    Decal->DecalSize = FVector(256.f, Range, Range);

    // 각도는 머티리얼 파라미터로 전달
    Decal->SetVisibility(true);
}

void APTSkillIndicatorActor::ShowSelfCircle(const FVector& Center, float Radius)
{
    ShowCircle(Center, Radius);
}

void APTSkillIndicatorActor::HideIndicator()
{
    Decal->SetVisibility(false);
}

void APTSkillIndicatorActor::BeginPlay()
{
    Super::BeginPlay();

    if (LineMaterial)   LineMID   = UMaterialInstanceDynamic::Create(LineMaterial,   this);
    if (CircleMaterial) CircleMID = UMaterialInstanceDynamic::Create(CircleMaterial, this);
    if (ConeMaterial)   ConeMID   = UMaterialInstanceDynamic::Create(ConeMaterial,   this);
}

