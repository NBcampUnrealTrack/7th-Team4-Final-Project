#include "PTLobbyPreviewActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

APTLobbyPreviewActor::APTLobbyPreviewActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;    // 로컬 전용

    PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
    RootComponent = PreviewMesh;

    CaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComp"));
    CaptureComp->SetupAttachment(RootComponent);
    CaptureComp->SetRelativeLocationAndRotation(FVector(300.f, 0.f, 90.f), FRotator(0.f, 180.f, 0.f));
    CaptureComp->CaptureSource         = ESceneCaptureSource::SCS_FinalColorLDR;
    CaptureComp->bCaptureEveryFrame    = true;
    CaptureComp->PrimitiveRenderMode   = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
}

void APTLobbyPreviewActor::BeginPlay()
{
    Super::BeginPlay();

    EnsureRenderTarget();

    if (CaptureComp)
    {
        CaptureComp->ShowOnlyActors.Empty();
        CaptureComp->ShowOnlyActors.Add(this);  // 본인만
    }
}

void APTLobbyPreviewActor::EnsureRenderTarget()
{
    if (RenderTarget)
    {
        return;
    }

    RenderTarget = NewObject<UTextureRenderTarget2D>(this);
    RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
    RenderTarget->InitAutoFormat(RenderTargetSize.X, RenderTargetSize.Y);
    RenderTarget->UpdateResourceImmediate(true);

    if (CaptureComp)
    {
        CaptureComp->TextureTarget = RenderTarget;
    }
}
