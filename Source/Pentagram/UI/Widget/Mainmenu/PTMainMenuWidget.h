#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PTMainMenuWidget.generated.h"

class UButton;
class UMediaPlayer;
class UMediaSource;

UCLASS()
class PENTAGRAM_API UPTMainMenuWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    void PlayMenuVideo();

    UFUNCTION()
    void HandleGameStartClicked();

    UFUNCTION()
    void HandleQuitClicked();

    UFUNCTION()
    void HandleJoinClicked();
protected:
    // 시작 버튼
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_GameStart;

    // 종료 버튼
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Quit;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Join;

    // 미디어 플레이어
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MainMenu")
    TObjectPtr<UMediaPlayer> MediaPlayer;

    // 메뉴 영상
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MainMenu")
    TObjectPtr<UMediaSource> MenuVideo;

    // 로비 레벨
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MainMenu")
    FName LobbyLevelName = TEXT("L_Lobby");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MainMenu")
    int32 MaxLobbyPlayers = 4;

};
