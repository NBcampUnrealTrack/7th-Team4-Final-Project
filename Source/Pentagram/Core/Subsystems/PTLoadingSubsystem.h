#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTLoadingSubsystem.generated.h"

class UDataTable;
class UPTLoadingWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTLoadingContextEvent, FName, LoadingContext);

UCLASS()
class PENTAGRAM_API UPTLoadingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void PreloadForTravel(
        const TArray<TSoftObjectPtr<UObject>>& AssetRefs,
        const TArray<TSoftClassPtr<UObject>>& ClassRefs,
        const TArray<UDataTable*>& DataTables,
        FSimpleDelegate OnComplete);

    UFUNCTION(BlueprintPure, Category = "PT|Loading")
    bool IsLoading() const { return bIsLoading; }

    UFUNCTION(BlueprintPure, Category = "PT|Loading")
    FName GetLoadingContext() const { return LoadingContext; }

    UFUNCTION(BlueprintPure, Category = "PT|Loading")
    float GetLoadingProgress() const;

    UPROPERTY(BlueprintAssignable, Category = "PT|Loading")
    FPTLoadingContextEvent OnLoadingStarted;

    UPROPERTY(BlueprintAssignable, Category = "PT|Loading")
    FPTLoadingContextEvent OnLoadingFinished;

private:
    void BeginLoading(FName InLoadingContext);
    void FinishLoading();

    void HandlePreLoadMap(const FString& MapName);
    void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);
    void HandlePreloadComplete();
    void ShowLoadingWidget();
    void HideLoadingWidget();

    void CollectDataTableSoftPaths(UDataTable* DataTable, TSet<FSoftObjectPath>& OutPaths) const;
    void CollectPropertySoftPaths(const FProperty* Property, const void* ValuePtr, TSet<FSoftObjectPath>& OutPaths) const;
    void AddSoftPath(const FSoftObjectPath& Path, TSet<FSoftObjectPath>& OutPaths) const;

private:
    FDelegateHandle PreLoadMapHandle;
    FDelegateHandle PostLoadMapHandle;

    FSimpleDelegate PendingPreloadComplete;
    TSharedPtr<FStreamableHandle> ActivePreloadHandle;

    UPROPERTY()
    TObjectPtr<UPTLoadingWidget> LoadingWidgetInstance;

    FName LoadingContext = NAME_None;
    bool bIsLoading = false;
};
