#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTLoadingWidget.generated.h"

class SProgressBar;
class STextBlock;
class UPTLoadingSubsystem;

UCLASS()
class PENTAGRAM_API UPTLoadingWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetLoadingContext(FName InLoadingContext);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPTLoadingSubsystem* ResolveLoadingSubsystem() const;
    void RefreshFromSubsystem();
    FText BuildStatusText(float Progress) const;

private:
    TSharedPtr<SProgressBar> SlateProgressBar;
    TSharedPtr<STextBlock> SlateStatusText;

    FName LoadingContext = NAME_None;
};
