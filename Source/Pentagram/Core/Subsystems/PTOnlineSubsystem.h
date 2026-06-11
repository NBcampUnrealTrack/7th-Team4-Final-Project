#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTOnlineSubsystem.generated.h"

UCLASS()
class PENTAGRAM_API UPTOnlineSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
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
};
