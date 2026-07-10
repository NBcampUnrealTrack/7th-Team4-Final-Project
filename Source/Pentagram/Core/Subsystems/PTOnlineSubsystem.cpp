#include "Core/Subsystems/PTOnlineSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

namespace
{
    constexpr int32 LocalUserNumber = 0;
}

void UPTOnlineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Login();

    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        SessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
            FOnSessionUserInviteAcceptedDelegate::CreateUObject(
                this,
                &UPTOnlineSubsystem::OnSessionUserInviteAccepted));
    }
}

void UPTOnlineSubsystem::Deinitialize()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem != nullptr)
    {
        IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
        if (IdentityInterface.IsValid() && LoginCompleteDelegateHandle.IsValid())
        {
            IdentityInterface->ClearOnLoginCompleteDelegate_Handle(
                LocalUserNumber,
                LoginCompleteDelegateHandle);
        }
    }

    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        if (CreateSessionCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        }

        if (DestroySessionCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        }

        if (JoinSessionCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        }

        if (SessionUserInviteAcceptedDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegateHandle);
        }
    }

    LoginCompleteDelegateHandle.Reset();
    CreateSessionCompleteDelegateHandle.Reset();
    DestroySessionCompleteDelegateHandle.Reset();
    JoinSessionCompleteDelegateHandle.Reset();
    SessionUserInviteAcceptedDelegateHandle.Reset();
    Super::Deinitialize();
}

void UPTOnlineSubsystem::Login()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem == nullptr)
    {
        OnLoginCompleted.Broadcast(false, FString(), FString(), TEXT("Online subsystem is unavailable."));
        return;
    }

    IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        OnLoginCompleted.Broadcast(false, FString(), FString(), TEXT("Identity interface is unavailable."));
        return;
    }

    if (IdentityInterface->GetLoginStatus(LocalUserNumber) == ELoginStatus::LoggedIn)
    {
        FUniqueNetIdPtr PlayerSteamID = IdentityInterface->GetUniquePlayerId(LocalUserNumber);
        OnLoginCompleted.Broadcast(
            true,
            IdentityInterface->GetPlayerNickname(LocalUserNumber),
            PlayerSteamID.IsValid() ? PlayerSteamID->ToString() : FString(),
            FString());
        return;
    }

    if (LoginCompleteDelegateHandle.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(
            LocalUserNumber,
            LoginCompleteDelegateHandle);
    }

    LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
        LocalUserNumber,
        FOnLoginCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnSteamLoginComplete));

    if (!IdentityInterface->AutoLogin(LocalUserNumber))
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(
            LocalUserNumber,
            LoginCompleteDelegateHandle);
        LoginCompleteDelegateHandle.Reset();
        OnLoginCompleted.Broadcast(false, FString(), FString(), TEXT("Steam AutoLogin failed."));
    }
}

void UPTOnlineSubsystem::Logout()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem == nullptr)
    {
        return;
    }

    IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        return;
    }

    IdentityInterface->Logout(LocalUserNumber);
}

bool UPTOnlineSubsystem::IsLoggedIn() const
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem == nullptr)
    {
        return false;
    }

    IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        return false;
    }

    return IdentityInterface->GetLoginStatus(LocalUserNumber) == ELoginStatus::LoggedIn;
}

FString UPTOnlineSubsystem::GetPlayerNickname() const
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem == nullptr)
    {
        return FString();
    }

    IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        return FString();
    }

    return IdentityInterface->GetPlayerNickname(LocalUserNumber);
}

FString UPTOnlineSubsystem::GetPlayerSteamID() const
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem == nullptr)
    {
        return FString();
    }

    IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        return FString();
    }

    FUniqueNetIdPtr PlayerSteamID = IdentityInterface->GetUniquePlayerId(LocalUserNumber);
    if (!PlayerSteamID.IsValid())
    {
        return FString();
    }

    return PlayerSteamID->ToString();
}

void UPTOnlineSubsystem::HostSteamSession(FName LobbyLevelName, int32 MaxPlayers, bool bShowInviteUIAfterCreate)
{
    if (LobbyLevelName.IsNone())
    {
        const FString ErrorMessage(TEXT("Lobby level name is None."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] HostSteamSession failed. %s"), *ErrorMessage);
        OnHostSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        const FString ErrorMessage(TEXT("Session interface is unavailable."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] HostSteamSession failed. %s"), *ErrorMessage);
        OnHostSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    PendingLobbyLevelName = LobbyLevelName;
    PendingMaxPlayers = FMath::Max(MaxPlayers, 1);
    bPendingInviteUIAfterCreate = bShowInviteUIAfterCreate;

    if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        bPendingCreateSessionAfterDestroy = true;

        if (DestroySessionCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        }

        DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnDestroySessionComplete));

        if (!SessionInterface->DestroySession(NAME_GameSession))
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
            DestroySessionCompleteDelegateHandle.Reset();
            bPendingCreateSessionAfterDestroy = false;

            const FString ErrorMessage(TEXT("Failed to destroy existing session."));
            UE_LOG(LogTemp, Warning, TEXT("[Online] HostSteamSession failed. %s"), *ErrorMessage);
            OnHostSessionCompleted.Broadcast(false, ErrorMessage);
        }
        return;
    }

    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = PendingMaxPlayers;
    SessionSettings.NumPrivateConnections = 0;
    SessionSettings.bIsLANMatch = false;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bAllowInvites = true;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bAllowJoinViaPresence = true;
    SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
    SessionSettings.bUseLobbiesIfAvailable = true;
    SessionSettings.Set(
        SETTING_MAPNAME,
        LobbyLevelName.ToString(),
        EOnlineDataAdvertisementType::ViaOnlineService);

    if (CreateSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnCreateSessionComplete));

    UE_LOG(LogTemp, Log, TEXT("[Online] Creating Steam session. Lobby=%s MaxPlayers=%d"),
        *LobbyLevelName.ToString(), PendingMaxPlayers);

    if (!SessionInterface->CreateSession(LocalUserNumber, NAME_GameSession, SessionSettings))
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        CreateSessionCompleteDelegateHandle.Reset();

        const FString ErrorMessage(TEXT("CreateSession request failed."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] HostSteamSession failed. %s"), *ErrorMessage);
        OnHostSessionCompleted.Broadcast(false, ErrorMessage);
    }
}

void UPTOnlineSubsystem::ShowSteamInviteUI()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] ShowSteamInviteUI failed. Online subsystem is unavailable."));
        return;
    }

    IOnlineExternalUIPtr ExternalUIInterface = OnlineSubsystem->GetExternalUIInterface();
    if (!ExternalUIInterface.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] ShowSteamInviteUI failed. External UI interface is unavailable."));
        return;
    }

    if (!ExternalUIInterface->ShowInviteUI(LocalUserNumber, NAME_GameSession))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] ShowSteamInviteUI failed. Steam invite UI was not shown."));
    }
}

void UPTOnlineSubsystem::DestroySteamSession()
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid() || SessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
    {
        return;
    }

    if (DestroySessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }

    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnDestroySessionComplete));
    SessionInterface->DestroySession(NAME_GameSession);
}

void UPTOnlineSubsystem::OnSteamLoginComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId& UserID,
    const FString& ErrorMessage)
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem == nullptr)
    {
        OnLoginCompleted.Broadcast(false, FString(), FString(), ErrorMessage);
        return;
    }

    IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        OnLoginCompleted.Broadcast(false, FString(), FString(), ErrorMessage);
        return;
    }

    IdentityInterface->ClearOnLoginCompleteDelegate_Handle(
        LocalUserNum,
        LoginCompleteDelegateHandle);
    LoginCompleteDelegateHandle.Reset();

    const FString PlayerNickname = bWasSuccessful
        ? IdentityInterface->GetPlayerNickname(LocalUserNum)
        : FString();
    const FString PlayerSteamID = bWasSuccessful
        ? UserID.ToString()
        : FString();

    OnLoginCompleted.Broadcast(
        bWasSuccessful,
        PlayerNickname,
        PlayerSteamID,
        ErrorMessage);
}

void UPTOnlineSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid() && CreateSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }
    CreateSessionCompleteDelegateHandle.Reset();

    if (!bWasSuccessful)
    {
        const FString ErrorMessage(TEXT("CreateSession completed with failure."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] %s Session=%s"), *ErrorMessage, *SessionName.ToString());
        OnHostSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Online] Steam session created. Session=%s"), *SessionName.ToString());
    OnHostSessionCompleted.Broadcast(true, FString());

    if (bPendingInviteUIAfterCreate)
    {
        ShowSteamInviteUI();
    }

    OpenPendingLobbyAsListenServer();
}

void UPTOnlineSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid() && DestroySessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }
    DestroySessionCompleteDelegateHandle.Reset();

    if (bPendingCreateSessionAfterDestroy)
    {
        bPendingCreateSessionAfterDestroy = false;

        if (!bWasSuccessful)
        {
            const FString ErrorMessage(TEXT("DestroySession completed with failure."));
            UE_LOG(LogTemp, Warning, TEXT("[Online] %s Session=%s"), *ErrorMessage, *SessionName.ToString());
            OnHostSessionCompleted.Broadcast(false, ErrorMessage);
            return;
        }

        HostSteamSession(PendingLobbyLevelName, PendingMaxPlayers, bPendingInviteUIAfterCreate);
    }
}

void UPTOnlineSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid() && JoinSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }
    JoinSessionCompleteDelegateHandle.Reset();

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        const FString ErrorMessage = FString::Printf(TEXT("JoinSession failed. Result=%d"), static_cast<int32>(Result));
        UE_LOG(LogTemp, Warning, TEXT("[Online] %s"), *ErrorMessage);
        OnJoinSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    FString ConnectInfo;
    if (!SessionInterface.IsValid() || !SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
    {
        const FString ErrorMessage(TEXT("Failed to resolve session connect string."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] %s Session=%s"), *ErrorMessage, *SessionName.ToString());
        OnJoinSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    APlayerController* PlayerController =
        GameInstance != nullptr ? GameInstance->GetFirstLocalPlayerController() : nullptr;
    if (PlayerController == nullptr)
    {
        const FString ErrorMessage(TEXT("Local PlayerController is unavailable."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] %s ConnectInfo=%s"), *ErrorMessage, *ConnectInfo);
        OnJoinSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Online] Joining Steam session. ConnectInfo=%s"), *ConnectInfo);
    PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
    OnJoinSessionCompleted.Broadcast(true, FString());
}

void UPTOnlineSubsystem::OnSessionUserInviteAccepted(
    const bool bWasSuccessful,
    const int32 ControllerId,
    FUniqueNetIdPtr UserId,
    const FOnlineSessionSearchResult& InviteResult)
{
    UE_LOG(LogTemp, Log, TEXT("[Online] Steam invite accepted. Success=%d ControllerId=%d"),
        bWasSuccessful ? 1 : 0, ControllerId);

    if (!bWasSuccessful)
    {
        OnJoinSessionCompleted.Broadcast(false, TEXT("Steam invite accept failed."));
        return;
    }

    JoinSteamSession(InviteResult);
}

IOnlineSessionPtr UPTOnlineSubsystem::GetSessionInterface() const
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    return OnlineSubsystem != nullptr ? OnlineSubsystem->GetSessionInterface() : nullptr;
}

void UPTOnlineSubsystem::OpenPendingLobbyAsListenServer()
{
    if (PendingLobbyLevelName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] Cannot open listen lobby. PendingLobbyLevelName is None."));
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] Cannot open listen lobby. World is null."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Online] Opening listen lobby. Level=%s"), *PendingLobbyLevelName.ToString());
    UGameplayStatics::OpenLevel(World, PendingLobbyLevelName, true, TEXT("listen"));
}

void UPTOnlineSubsystem::JoinSteamSession(const FOnlineSessionSearchResult& SearchResult)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        OnJoinSessionCompleted.Broadcast(false, TEXT("Session interface is unavailable."));
        return;
    }

    if (JoinSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }

    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnJoinSessionComplete));

    if (!SessionInterface->JoinSession(LocalUserNumber, NAME_GameSession, SearchResult))
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        JoinSessionCompleteDelegateHandle.Reset();
        OnJoinSessionCompleted.Broadcast(false, TEXT("JoinSession request failed."));
    }
}
