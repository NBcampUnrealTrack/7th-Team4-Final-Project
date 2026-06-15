#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTOnlineSubsystem.generated.h"

class FUniqueNetId;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FPTLoginCompletedDelegate,
    bool, bWasSuccessful,
    FString, PlayerNickname,
    FString, PlayerUniqueID,
    FString, ErrorMessage);

UCLASS()
class PENTAGRAM_API UPTOnlineSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
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
    FString GetPlayerUniqueID() const;

private:
    void OnSteamLoginComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FUniqueNetId& UserID,
        const FString& ErrorMessage);

    FDelegateHandle LoginCompleteDelegateHandle;

public:
    UPROPERTY(BlueprintAssignable, Category = "PT|Online")
    FPTLoginCompletedDelegate OnLoginCompleted;
};
