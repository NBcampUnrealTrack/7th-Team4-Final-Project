#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTOnlineSubsystem.generated.h"

class FUniqueNetId;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FPTLoginCompletedDelegate,
    bool, bWasSuccessful,
    FString, PlayerNickname,
    FString, PlayerSteamID,
    FString, ErrorMessage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FPTOnlineSessionEvent,
    bool, bWasSuccessful,
    FString, ErrorMessage);

UCLASS()
class PENTAGRAM_API UPTOnlineSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "PT|Online")
    void Login();

    UFUNCTION(BlueprintCallable, Category = "PT|Online")
    void Logout();

    UFUNCTION(BlueprintPure, Category = "PT|Online")
    bool IsLoggedIn() const;

    UFUNCTION(BlueprintPure, Category = "PT|Online")
    FString GetPlayerNickname() const;

    UFUNCTION(BlueprintPure, Category = "PT|Online")
    FString GetPlayerSteamID() const;

    UFUNCTION(BlueprintCallable, Category = "PT|Online|Session")
    void HostSteamSession(FName LobbyLevelName = TEXT("L_Lobby"), int32 MaxPlayers = 4, bool bShowInviteUIAfterCreate = true);

    UFUNCTION(BlueprintCallable, Category = "PT|Online|Session")
    void ShowSteamInviteUI();

    UFUNCTION(BlueprintCallable, Category = "PT|Online|Session")
    void DestroySteamSession();

private:
    void OnSteamLoginComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FUniqueNetId& UserID,
        const FString& ErrorMessage);

    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnSessionUserInviteAccepted(
        const bool bWasSuccessful,
        const int32 ControllerId,
        FUniqueNetIdPtr UserId,
        const FOnlineSessionSearchResult& InviteResult);

    IOnlineSessionPtr GetSessionInterface() const;
    void OpenPendingLobbyAsListenServer();
    void JoinSteamSession(const FOnlineSessionSearchResult& SearchResult);

    FDelegateHandle LoginCompleteDelegateHandle;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FDelegateHandle DestroySessionCompleteDelegateHandle;
    FDelegateHandle JoinSessionCompleteDelegateHandle;
    FDelegateHandle SessionUserInviteAcceptedDelegateHandle;

    FName PendingLobbyLevelName = NAME_None;
    bool bPendingInviteUIAfterCreate = false;
    bool bPendingCreateSessionAfterDestroy = false;
    int32 PendingMaxPlayers = 4;

public:
    UPROPERTY(BlueprintAssignable, Category = "PT|Online")
    FPTLoginCompletedDelegate OnLoginCompleted;

    UPROPERTY(BlueprintAssignable, Category = "PT|Online|Session")
    FPTOnlineSessionEvent OnHostSessionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "PT|Online|Session")
    FPTOnlineSessionEvent OnJoinSessionCompleted;
};
