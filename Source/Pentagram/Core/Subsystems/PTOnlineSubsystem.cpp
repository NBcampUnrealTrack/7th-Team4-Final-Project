#include "Core/Subsystems/PTOnlineSubsystem.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"

namespace
{
    constexpr int32 LocalUserNumber = 0;
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

    LoginCompleteDelegateHandle.Reset();
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

    if (LoginCompleteDelegateHandle.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(
            LocalUserNumber,
            LoginCompleteDelegateHandle);
    }

    LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
        LocalUserNumber,
        FOnLoginCompleteDelegate::CreateUObject(this, &UPTOnlineSubsystem::OnSteamLoginComplete));

    IdentityInterface->AutoLogin(LocalUserNumber);
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

FString UPTOnlineSubsystem::GetPlayerUniqueID() const
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

    FUniqueNetIdPtr PlayerUniqueID = IdentityInterface->GetUniquePlayerId(LocalUserNumber);
    if (!PlayerUniqueID.IsValid())
    {
        return FString();
    }

    return PlayerUniqueID->ToString();
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
    const FString PlayerUniqueID = bWasSuccessful
        ? UserID.ToString()
        : FString();

    OnLoginCompleted.Broadcast(
        bWasSuccessful,
        PlayerNickname,
        PlayerUniqueID,
        ErrorMessage);
}
