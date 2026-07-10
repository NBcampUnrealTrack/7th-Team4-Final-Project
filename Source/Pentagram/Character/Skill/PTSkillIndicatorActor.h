#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTSkillIndicatorActor.generated.h"

UCLASS()
class PENTAGRAM_API APTSkillIndicatorActor : public AActor
{
    GENERATED_BODY()
public:
    APTSkillIndicatorActor();

    void ShowLine(const FVector& Origin, const FVector& Dir, float Range, float Width);

    void ShowCircle(const FVector& Center, float Radius);

    void ShowCone(const FVector& Origin, const FVector& Dir, float Range, float AngleDeg);

    void ShowSelfCircle(const FVector& Center, float Radius);

    void HideIndicator();

    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<class USceneComponent>  Root;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UDecalComponent>  Decal;

    // 모양별 머티리얼 (에디터에서 지정)
    UPROPERTY(EditAnywhere, Category="Indicator") TObjectPtr<UMaterialInterface> LineMaterial; // 라인
    UPROPERTY(EditAnywhere, Category="Indicator") TObjectPtr<UMaterialInterface> CircleMaterial; // 원
    UPROPERTY(EditAnywhere, Category="Indicator") TObjectPtr<UMaterialInterface> ConeMaterial; // 부채꼴

    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> LineMID;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> CircleMID;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> ConeMID;
};
