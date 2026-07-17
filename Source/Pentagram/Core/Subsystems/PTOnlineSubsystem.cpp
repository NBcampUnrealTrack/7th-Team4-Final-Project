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
    const FName LobbyUILevelSettingName(TEXT("PT_LOBBY_UI_LEVEL"));
    const FName FrontendWorldLevelName(TEXT("L_Intro"));
    const FName DefaultMainMenuUILevelName(TEXT("L_MainMenu"));
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

    UWorld* World = GetWorld();
    const FString HostLevelName = World != nullptr
        ? UGameplayStatics::GetCurrentLevelName(World, true)
        : FString();
    if (HostLevelName.IsEmpty())
    {
        const FString ErrorMessage(TEXT("Current host level is unavailable."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] HostSteamSession failed. %s"), *ErrorMessage);
        OnHostSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    PendingLobbyLevelName = LobbyLevelName;
    PendingHostLevelName = FName(*HostLevelName);
    PendingMaxPlayers = FMath::Max(MaxPlayers, 1);
    bPendingInviteUIAfterCreate = bShowInviteUIAfterCreate;

    if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        PendingSessionAction = EPendingSessionAction::CreateSession;
        PendingInviteResult.Reset();
        DestroySteamSession();
        return;
    }

    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = PendingMaxPlayers;
    SessionSettings.NumPrivateConnections = 0;
    SessionSettings.bIsLANMatch = false;
    SessionSettings.bIsDedicated = false;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bAllowInvites = true;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bAllowJoinViaPresence = true;
    SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
    SessionSettings.bUseLobbiesIfAvailable = true;
    SessionSettings.Set(
        SETTING_MAPNAME,
        PendingHostLevelName.ToString(),
        EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings.Set(
        LobbyUILevelSettingName,
        LobbyLevelName.ToString(),
        EOnlineDataAdvertisementType::ViaOnlineService);

    if (CreateSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnCreateSessionComplete));

    UE_LOG(LogTemp, Log, TEXT("[Online] Creating Steam session. HostLevel=%s LobbyUI=%s MaxPlayers=%d"),
        *PendingHostLevelName.ToString(), *LobbyLevelName.ToString(), PendingMaxPlayers);

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

    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid() || SessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] ShowSteamInviteUI failed. Game session does not exist."));
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
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] DestroySession failed. Session interface is unavailable."));
        HandleDestroySessionFinished(false, NAME_GameSession);
        return;
    }

    if (SessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
    {
        HandleDestroySessionFinished(true, NAME_GameSession);
        return;
    }

    if (DestroySessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }

    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnDestroySessionComplete));

    UE_LOG(LogTemp, Log, TEXT("[Online] Destroying Steam session."));
    if (!SessionInterface->DestroySession(NAME_GameSession))
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        DestroySessionCompleteDelegateHandle.Reset();
        UE_LOG(LogTemp, Warning, TEXT("[Online] DestroySession request failed."));
        HandleDestroySessionFinished(false, NAME_GameSession);
    }
}

void UPTOnlineSubsystem::LeaveSteamSession(FName MainMenuUILevelName)
{
    PendingReturnUILevelName = MainMenuUILevelName.IsNone()
        ? DefaultMainMenuUILevelName
        : MainMenuUILevelName;
    PendingSessionAction = EPendingSessionAction::ReturnToMainMenu;
    PendingInviteResult.Reset();
    PendingJoinLobbyLevelName = NAME_None;
    PendingSessionConnectString.Reset();
    PendingSessionConnectLobbyLevelName = NAME_None;

    DestroySteamSession();
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

    HandleDestroySessionFinished(bWasSuccessful, SessionName);
}

void UPTOnlineSubsystem::HandleDestroySessionFinished(bool bWasSuccessful, FName SessionName)
{
    const EPendingSessionAction CompletedAction = PendingSessionAction;
    PendingSessionAction = EPendingSessionAction::None;

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("[Online] DestroySession finished. Session=%s Action=%d"),
            *SessionName.ToString(), static_cast<int32>(CompletedAction));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] DestroySession failed. Session=%s Action=%d"),
            *SessionName.ToString(), static_cast<int32>(CompletedAction));
    }

    switch (CompletedAction)
    {
    case EPendingSessionAction::CreateSession:
        if (bWasSuccessful)
        {
            HostSteamSession(PendingLobbyLevelName, PendingMaxPlayers, bPendingInviteUIAfterCreate);
        }
        else
        {
            OnHostSessionCompleted.Broadcast(false, TEXT("Failed to destroy the existing session."));
        }
        break;

    case EPendingSessionAction::JoinInvite:
        if (bWasSuccessful && PendingInviteResult.IsValid())
        {
            const FOnlineSessionSearchResult InviteResult = *PendingInviteResult;
            PendingInviteResult.Reset();
            JoinSteamSession(InviteResult);
        }
        else
        {
            PendingInviteResult.Reset();
            OnJoinSessionCompleted.Broadcast(false, TEXT("Failed to leave the existing session before joining the invite."));
        }
        break;

    case EPendingSessionAction::ReturnToMainMenu:
        ReturnToMainMenu();
        break;

    case EPendingSessionAction::None:
    default:
        break;
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
        PendingJoinLobbyLevelName = NAME_None;
        PendingSessionConnectString.Reset();
        PendingSessionConnectLobbyLevelName = NAME_None;
        const FString ErrorMessage = FString::Printf(TEXT("JoinSession failed. Result=%d"), static_cast<int32>(Result));
        UE_LOG(LogTemp, Warning, TEXT("[Online] %s"), *ErrorMessage);
        OnJoinSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    FString ConnectInfo;
    if (!SessionInterface.IsValid() || !SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
    {
        PendingJoinLobbyLevelName = NAME_None;
        PendingSessionConnectString.Reset();
        PendingSessionConnectLobbyLevelName = NAME_None;
        const FString ErrorMessage(TEXT("Failed to resolve session connect string."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] %s Session=%s"), *ErrorMessage, *SessionName.ToString());
        OnJoinSessionCompleted.Broadcast(false, ErrorMessage);
        return;
    }

    const FName LobbyUILevelName = PendingJoinLobbyLevelName.IsNone()
        ? FName(TEXT("L_Lobby"))
        : PendingJoinLobbyLevelName;
    PendingJoinLobbyLevelName = NAME_None;

    UE_LOG(LogTemp, Log, TEXT("[Online] Session address resolved. Session=%s ConnectInfo=%s LobbyUI=%s"),
        *SessionName.ToString(), *ConnectInfo, *LobbyUILevelName.ToString());

    UGameInstance* GameInstance = GetGameInstance();
    APlayerController* PlayerController =
        GameInstance != nullptr ? GameInstance->GetFirstLocalPlayerController() : nullptr;
    if (PlayerController == nullptr)
    {
        PendingSessionConnectString = MoveTemp(ConnectInfo);
        PendingSessionConnectLobbyLevelName = LobbyUILevelName;
        UE_LOG(LogTemp, Log, TEXT("[Online] PlayerController is not ready. Session travel was deferred."));
        OnJoinSessionCompleted.Broadcast(true, FString());
        return;
    }

    TravelToResolvedSession(PlayerController, ConnectInfo, LobbyUILevelName);
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

    if (!bWasSuccessful || !InviteResult.IsValid())
    {
        OnJoinSessionCompleted.Broadcast(false, TEXT("Steam invite accept failed."));
        return;
    }

    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        OnJoinSessionCompleted.Broadcast(false, TEXT("Session interface is unavailable."));
        return;
    }

    if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        PendingInviteResult = MakeShared<FOnlineSessionSearchResult>(InviteResult);
        PendingSessionAction = EPendingSessionAction::JoinInvite;
        UE_LOG(LogTemp, Log, TEXT("[Online] Existing session found. Leaving it before joining the invite."));
        DestroySteamSession();
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
    if (PendingLobbyLevelName.IsNone() || PendingHostLevelName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] Cannot open listen lobby. HostLevel=%s LobbyUI=%s"),
            *PendingHostLevelName.ToString(), *PendingLobbyLevelName.ToString());
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] Cannot open listen lobby. World is null."));
        return;
    }

    PendingLocalUILevelName = PendingLobbyLevelName;

    UE_LOG(LogTemp, Log, TEXT("[Online] Opening listen server. HostLevel=%s LobbyUI=%s"),
        *PendingHostLevelName.ToString(), *PendingLobbyLevelName.ToString());
    UGameplayStatics::OpenLevel(World, PendingHostLevelName, true, TEXT("listen"));
}

void UPTOnlineSubsystem::JoinSteamSession(const FOnlineSessionSearchResult& SearchResult)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        OnJoinSessionCompleted.Broadcast(false, TEXT("Session interface is unavailable."));
        return;
    }

    FString LobbyUILevelName;
    if (!SearchResult.Session.SessionSettings.Get(LobbyUILevelSettingName, LobbyUILevelName))
    {
        // 구버전 세션은 SETTING_MAPNAME에 L_Lobby를 저장했으므로 호환용으로 사용한다.
        SearchResult.Session.SessionSettings.Get(SETTING_MAPNAME, LobbyUILevelName);
    }
    PendingJoinLobbyLevelName = LobbyUILevelName.IsEmpty()
        ? FName(TEXT("L_Lobby"))
        : FName(*LobbyUILevelName);

    if (JoinSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }

    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnJoinSessionComplete));

    UE_LOG(LogTemp, Log, TEXT("[Online] Requesting JoinSession. LobbyUI=%s"),
        *PendingJoinLobbyLevelName.ToString());
    if (!SessionInterface->JoinSession(LocalUserNumber, NAME_GameSession, SearchResult))
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        JoinSessionCompleteDelegateHandle.Reset();
        PendingJoinLobbyLevelName = NAME_None;
        const FString ErrorMessage(TEXT("JoinSession request failed."));
        UE_LOG(LogTemp, Warning, TEXT("[Online] %s"), *ErrorMessage);
        OnJoinSessionCompleted.Broadcast(false, ErrorMessage);
    }
}

FName UPTOnlineSubsystem::ConsumePendingLocalUILevelName()
{
    const FName UILevelName = PendingLocalUILevelName;
    PendingLocalUILevelName = NAME_None;

    if (!UILevelName.IsNone())
    {
        UE_LOG(LogTemp, Log, TEXT("[Online] Consuming pending local UI level. Level=%s"),
            *UILevelName.ToString());
    }

    return UILevelName;
}

bool UPTOnlineSubsystem::TravelToPendingSessionJoin(APlayerController* PlayerController)
{
    if (PlayerController == nullptr || PendingSessionConnectString.IsEmpty())
    {
        return false;
    }

    const FString ConnectInfo = MoveTemp(PendingSessionConnectString);
    const FName LobbyUILevelName = PendingSessionConnectLobbyLevelName.IsNone()
        ? FName(TEXT("L_Lobby"))
        : PendingSessionConnectLobbyLevelName;
    PendingSessionConnectString.Reset();
    PendingSessionConnectLobbyLevelName = NAME_None;

    UE_LOG(LogTemp, Log, TEXT("[Online] Continuing deferred session travel."));
    TravelToResolvedSession(PlayerController, ConnectInfo, LobbyUILevelName);
    return true;
}

void UPTOnlineSubsystem::TravelToResolvedSession(
    APlayerController* PlayerController,
    const FString& ConnectInfo,
    FName LobbyUILevelName)
{
    if (PlayerController == nullptr || ConnectInfo.IsEmpty())
    {
        return;
    }

    PendingLocalUILevelName = LobbyUILevelName;
    UE_LOG(LogTemp, Log, TEXT("[Online] Starting session ClientTravel. ConnectInfo=%s LobbyUI=%s"),
        *ConnectInfo, *LobbyUILevelName.ToString());
    PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
}

void UPTOnlineSubsystem::ReturnToMainMenu()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Online] Cannot return to main menu. World is null."));
        return;
    }

    PendingLocalUILevelName = PendingReturnUILevelName.IsNone()
        ? DefaultMainMenuUILevelName
        : PendingReturnUILevelName;
    PendingReturnUILevelName = NAME_None;

    UE_LOG(LogTemp, Log, TEXT("[Online] Returning to frontend. World=%s UI=%s"),
        *FrontendWorldLevelName.ToString(), *PendingLocalUILevelName.ToString());
    UGameplayStatics::OpenLevel(World, FrontendWorldLevelName);
}
