#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTLobbyPreviewActor.generated.h"

class USkeletalMeshComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class PENTAGRAM_API APTLobbyPreviewActor : public AActor
{
    GENERATED_BODY()

public:
    APTLobbyPreviewActor();

    virtual void BeginPlay() override;
    UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

protected:
    void EnsureRenderTarget();  // RT 생성

    UPROPERTY(VisibleAnywhere, Category = "PT|Preview")
    TObjectPtr<USkeletalMeshComponent> PreviewMesh;

    UPROPERTY(VisibleAnywhere, Category = "PT|Preview")
    TObjectPtr<USceneCaptureComponent2D> CaptureComp;

    UPROPERTY(EditAnywhere, Category = "PT|Preview")
    FIntPoint RenderTargetSize = FIntPoint(512, 1024);

private:

    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> RenderTarget;
};
