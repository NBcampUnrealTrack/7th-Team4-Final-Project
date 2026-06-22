#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PTGoldWidget.generated.h"

class UTextBlock;
class UImage;
class UTexture2D;
class APTBasePlayerState;

UCLASS()
class PENTAGRAM_API UPTGoldWidget : public UCommonUserWidget
{
    GENERATED_BODY()

protected:
    // 오버라이드
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 스테이트 바인딩
    void BindToPlayerState();

    // 아이콘 적용
    void ApplyGoldIcon();

    // 골드 갱신
    UFUNCTION()
    void HandleGoldChanged(int64 NewAmount);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Gold;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_GoldIcon;

    // 골드 아이콘
    UPROPERTY(EditAnywhere, Category = "PT|Gold")
    TObjectPtr<UTexture2D> GoldIcon;

private:
    // 바인딩 대상
    TWeakObjectPtr<APTBasePlayerState> BoundState;

    // 재시도 타이머
    FTimerHandle BindRetryTimer;
};
