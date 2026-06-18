#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTLobbyWidget.generated.h"

class UPTLobbySlotWidget;
class UButton;
class UTextBlock;
class UImage;
class APTLobbyPreviewActor;
class UMaterialInterface;

UCLASS()
class PENTAGRAM_API UPTLobbyWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    // ── 오버라이드 함수 ──────────────────────────────────────────────────────

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ── 일반 멤버 함수 ───────────────────────────────────────────────────────

    void BindGameState();   // GS 바인드
    void RefreshLobby();    // 목록 갱신
    void SpawnPreview();    // 프리뷰 스폰
    void RefreshPreview();  // 프리뷰 갱신

    UFUNCTION()
    void HandleLobbyUpdated();

    UFUNCTION()
    void OnReadyClicked();

protected:
    // ── 멤버 변수 ────────────────────────────────────────────────────────────

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPTLobbySlotWidget> PlayerSlot_0;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPTLobbySlotWidget> PlayerSlot_1;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPTLobbySlotWidget> PlayerSlot_2;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPTLobbySlotWidget> PlayerSlot_3;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPTLobbySlotWidget> PlayerSlot_4;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ReadyButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ReadyButtonText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> CharacterPreviewImage;

    UPROPERTY(EditAnywhere, Category = "PT|Lobby")
    TSubclassOf<APTLobbyPreviewActor> PreviewActorClass;

    UPROPERTY(EditAnywhere, Category = "PT|Lobby")
    FTransform PreviewSpawnTransform;

    UPROPERTY(EditAnywhere, Category = "PT|Lobby")
    TObjectPtr<UMaterialInterface> PreviewMaterial;
private:
    // ── 멤버 변수 (private) ──────────────────────────────────────────────────

    bool bLocalReady = false;

    FTimerHandle BindRetryTimer;

    UPROPERTY()
    TObjectPtr<APTLobbyPreviewActor> PreviewActor;
};
