#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTLoadingWidget.generated.h"

class UImage;
class UBorder;
class UWidget;
class UTextBlock;
class UPTLoadingSubsystem;
class UTexture2D;

UCLASS()
class PENTAGRAM_API UPTLoadingWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetLoadingContext(FName InLoadingContext);

    UFUNCTION(BlueprintPure, Category = "PT|Loading")
    float GetLoadingProgress() const;

    UFUNCTION(BlueprintPure, Category = "PT|Loading")
    FText GetLoadingStatusText() const;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPTLoadingSubsystem* ResolveLoadingSubsystem() const;
    void LoadBackgroundTextures();
    void ApplyBackgroundTexture();
    void AdvanceBackgroundTexture(float DeltaTime);
    void SelectLoadingTip();
    void RefreshFromSubsystem();
    void RefreshBoundWidgets(float Progress);
    void ApplyFullscreenCanvasSlot(UWidget* Widget, int32 ZOrder) const;
    void HideLegacyWidgets();
    FText BuildStatusText(float Progress) const;
    FText BuildTipText() const;

private:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_Background;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> Border_Dim;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_ProgressFill;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_ProgressFrame;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_DividerLine;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_Compass;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_LoadingStatus;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Status;

    UPROPERTY(Transient)
    TObjectPtr<UTexture2D> BackgroundTexture;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTexture2D>> BackgroundTextures;

    int32 BackgroundTextureIndex = INDEX_NONE;

    float BackgroundCycleElapsedTime = 0.f;

    float BackgroundCycleInterval = 3.f;

    FText CurrentTipText;

    FName LoadingContext = NAME_None;
};
