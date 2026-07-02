#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "TimerManager.h"
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

    void PreloadForStartup(
        FName TargetLevelName,
        float MinDisplaySeconds,
        const TArray<TSoftObjectPtr<UObject>>& AdditionalAssetRefs,
        const TArray<TSoftClassPtr<UObject>>& AdditionalClassRefs);

    UFUNCTION(BlueprintPure, Category = "PT|Loading")
    bool IsLoading() const { return bIsLoading; }

    UFUNCTION(BlueprintPure, Category = "PT|Loading")
    FName GetLoadingContext() const { return LoadingContext; }

    UFUNCTION(BlueprintPure, Category = "PT|Loading")
    float GetLoadingProgress() const;

    UFUNCTION(BlueprintCallable, Category = "PT|Loading")
    void FinishLoading();

    UPROPERTY(BlueprintAssignable, Category = "PT|Loading")
    FPTLoadingContextEvent OnLoadingStarted;

    UPROPERTY(BlueprintAssignable, Category = "PT|Loading")
    FPTLoadingContextEvent OnLoadingFinished;

private:
    void BeginLoading(FName InLoadingContext, bool bShowLoadingWidget = true);

    void HandlePreLoadMap(const FString& MapName);
    void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);
    void HandlePreloadComplete();
    void HandleStartupPreloadComplete();
    void OpenStartupTargetLevel();
    void ReleaseRetainedPreloadHandle();
    void ShowLoadingWidget();
    void HideLoadingWidget();

    void CollectStartupPreloadPaths(
        const TArray<TSoftObjectPtr<UObject>>& AdditionalAssetRefs,
        const TArray<TSoftClassPtr<UObject>>& AdditionalClassRefs,
        TSet<FSoftObjectPath>& OutPaths) const;
    void CollectDataTableSoftPaths(UDataTable* DataTable, TSet<FSoftObjectPath>& OutPaths) const;
    void CollectPropertySoftPaths(const FProperty* Property, const void* ValuePtr, TSet<FSoftObjectPath>& OutPaths) const;
    void AddSoftPath(const FSoftObjectPath& Path, TSet<FSoftObjectPath>& OutPaths) const;

private:
    FDelegateHandle PreLoadMapHandle;
    FDelegateHandle PostLoadMapHandle;

    FSimpleDelegate PendingPreloadComplete;
    TSharedPtr<FStreamableHandle> ActivePreloadHandle;
    TSharedPtr<FStreamableHandle> RetainedPreloadHandle;
    FTimerHandle ReleasePreloadHandleTimerHandle;
    FTimerHandle StartupTravelTimerHandle;

    UPROPERTY()
    TObjectPtr<UPTLoadingWidget> LoadingWidgetInstance;

    FName StartupTargetLevelName = NAME_None;
    FName LoadingContext = NAME_None;
    double StartupBeginTime = 0.0;
    float StartupMinDisplaySeconds = 0.f;
    bool bIsLoading = false;
};
