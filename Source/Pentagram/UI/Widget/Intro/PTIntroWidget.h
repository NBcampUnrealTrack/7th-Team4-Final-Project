#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTIntroWidget.generated.h"

class UMediaPlayer;
class UMediaSource;

UCLASS()
class PENTAGRAM_API UPTIntroWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

    void Play();
    void GoToMenu();

    UFUNCTION()
    void HandleEndReached();

protected:
    // 미디어 플레이어
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
    TObjectPtr<UMediaPlayer> MediaPlayer;

    // 1번 영상
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
    TObjectPtr<UMediaSource> Video01;

    // 2번 영상
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
    TObjectPtr<UMediaSource> Video02;

    // 다음 레벨
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
    FName MenuLevelName;

    // 다음 UI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
    TSubclassOf<UCommonActivatableWidget> MenuWidgetClass;

private:
    bool bLooping = false;
    bool bTransitioned = false;
};
