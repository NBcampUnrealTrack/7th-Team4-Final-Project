#include "Core/Subsystems/PTLoadingSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UI/Setting/PTUISettings.h"
#include "UI/Widget/Loading/PTLoadingWidget.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UnrealType.h"

namespace
{
constexpr float PreloadHandleRetentionSeconds = 30.f;
const FName StartupLoadingContext(TEXT("Startup"));
}

void UPTLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(
        this,
        &UPTLoadingSubsystem::HandlePreLoadMap);
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
        this,
        &UPTLoadingSubsystem::HandlePostLoadMapWithWorld);
}

void UPTLoadingSubsystem::Deinitialize()
{
    if (ActivePreloadHandle.IsValid())
    {
        ActivePreloadHandle->CancelHandle();
        ActivePreloadHandle.Reset();
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ReleasePreloadHandleTimerHandle);
        World->GetTimerManager().ClearTimer(StartupTravelTimerHandle);
    }
    RetainedPreloadHandle.Reset();

    PendingPreloadComplete.Unbind();
    HideLoadingWidget();

    if (PreLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
        PreLoadMapHandle.Reset();
    }

    if (PostLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
        PostLoadMapHandle.Reset();
    }

    Super::Deinitialize();
}

void UPTLoadingSubsystem::PreloadForTravel(
    const TArray<TSoftObjectPtr<UObject>>& AssetRefs,
    const TArray<TSoftClassPtr<UObject>>& ClassRefs,
    const TArray<UDataTable*>& DataTables,
    FSimpleDelegate OnComplete)
{
    BeginLoading(TEXT("TravelPreload"));

    if (ActivePreloadHandle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Loading] Replacing an active preload request."));
        ActivePreloadHandle->CancelHandle();
        ActivePreloadHandle.Reset();
    }

    if (PendingPreloadComplete.IsBound())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Loading] Replacing a pending preload completion delegate."));
        PendingPreloadComplete.Unbind();
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ReleasePreloadHandleTimerHandle);
    }
    RetainedPreloadHandle.Reset();

    PendingPreloadComplete = OnComplete;

    TSet<FSoftObjectPath> UniquePaths;
    for (const TSoftObjectPtr<UObject>& AssetRef : AssetRefs)
    {
        AddSoftPath(AssetRef.ToSoftObjectPath(), UniquePaths);
    }

    for (const TSoftClassPtr<UObject>& ClassRef : ClassRefs)
    {
        AddSoftPath(ClassRef.ToSoftObjectPath(), UniquePaths);
    }

    for (UDataTable* DataTable : DataTables)
    {
        CollectDataTableSoftPaths(DataTable, UniquePaths);
    }

    UE_LOG(LogTemp, Log, TEXT("[Loading] Collected preload paths: %d"), UniquePaths.Num());

    if (UniquePaths.IsEmpty())
    {
        HandlePreloadComplete();
        return;
    }

    TArray<FSoftObjectPath> PathsToLoad = UniquePaths.Array();
    ActivePreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        PathsToLoad,
        FStreamableDelegate::CreateUObject(this, &UPTLoadingSubsystem::HandlePreloadComplete),
        FStreamableManager::AsyncLoadHighPriority,
        false,
        false,
        TEXT("PTTravelPreload"));
}

void UPTLoadingSubsystem::PreloadForStartup(
    FName TargetLevelName,
    float MinDisplaySeconds,
    const TArray<TSoftObjectPtr<UObject>>& AdditionalAssetRefs,
    const TArray<TSoftClassPtr<UObject>>& AdditionalClassRefs)
{
    if (TargetLevelName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Startup] TargetLevelName is None."));
        return;
    }

    BeginLoading(StartupLoadingContext, false);

    if (ActivePreloadHandle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Startup] Replacing an active preload request."));
        ActivePreloadHandle->CancelHandle();
        ActivePreloadHandle.Reset();
    }

    if (PendingPreloadComplete.IsBound())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Startup] Clearing a pending preload completion delegate."));
        PendingPreloadComplete.Unbind();
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(StartupTravelTimerHandle);
        World->GetTimerManager().ClearTimer(ReleasePreloadHandleTimerHandle);
    }
    RetainedPreloadHandle.Reset();

    StartupTargetLevelName = TargetLevelName;
    StartupBeginTime = FPlatformTime::Seconds();
    StartupMinDisplaySeconds = FMath::Max(MinDisplaySeconds, 0.f);

    TSet<FSoftObjectPath> UniquePaths;
    CollectStartupPreloadPaths(AdditionalAssetRefs, AdditionalClassRefs, UniquePaths);

    UE_LOG(LogTemp, Log, TEXT("[Startup] Preload paths: %d"), UniquePaths.Num());

    if (UniquePaths.IsEmpty())
    {
        HandleStartupPreloadComplete();
        return;
    }

    TArray<FSoftObjectPath> PathsToLoad = UniquePaths.Array();
    ActivePreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        PathsToLoad,
        FStreamableDelegate::CreateUObject(this, &UPTLoadingSubsystem::HandleStartupPreloadComplete),
        FStreamableManager::AsyncLoadHighPriority,
        false,
        false,
        TEXT("PTStartupPreload"));
}

float UPTLoadingSubsystem::GetLoadingProgress() const
{
    if (ActivePreloadHandle.IsValid())
    {
        return ActivePreloadHandle->GetProgress();
    }

    return bIsLoading ? 0.f : 1.f;
}

void UPTLoadingSubsystem::BeginLoading(FName InLoadingContext, bool bShowLoadingWidget)
{
    LoadingContext = InLoadingContext;
    if (bIsLoading)
    {
        if (LoadingWidgetInstance != nullptr)
        {
            LoadingWidgetInstance->SetLoadingContext(LoadingContext);
        }
        else if (bShowLoadingWidget)
        {
            ShowLoadingWidget();
        }
        return;
    }

    bIsLoading = true;
    if (bShowLoadingWidget)
    {
        ShowLoadingWidget();
    }
    OnLoadingStarted.Broadcast(LoadingContext);
}

void UPTLoadingSubsystem::FinishLoading()
{
    if (!bIsLoading)
    {
        return;
    }

    if (ActivePreloadHandle.IsValid())
    {
        RetainedPreloadHandle = ActivePreloadHandle;
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ReleasePreloadHandleTimerHandle,
                this,
                &UPTLoadingSubsystem::ReleaseRetainedPreloadHandle,
                PreloadHandleRetentionSeconds,
                false);
        }
    }
    ActivePreloadHandle.Reset();

    const FName FinishedContext = LoadingContext;
    LoadingContext = NAME_None;
    bIsLoading = false;
    OnLoadingFinished.Broadcast(FinishedContext);
    HideLoadingWidget();
}

void UPTLoadingSubsystem::HandlePreLoadMap(const FString& MapName)
{
    if (LoadingContext == StartupLoadingContext)
    {
        return;
    }

    BeginLoading(TEXT("MapTravel"));
}

void UPTLoadingSubsystem::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
    FinishLoading();
}

void UPTLoadingSubsystem::HandlePreloadComplete()
{
    FSimpleDelegate CompletionDelegate = PendingPreloadComplete;
    PendingPreloadComplete.Unbind();

    if (CompletionDelegate.IsBound())
    {
        CompletionDelegate.Execute();
    }
}

void UPTLoadingSubsystem::HandleStartupPreloadComplete()
{
    const double ElapsedSeconds = FPlatformTime::Seconds() - StartupBeginTime;
    const float RemainingSeconds = FMath::Max(StartupMinDisplaySeconds - static_cast<float>(ElapsedSeconds), 0.f);

    if (RemainingSeconds <= 0.f)
    {
        OpenStartupTargetLevel();
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            StartupTravelTimerHandle,
            this,
            &UPTLoadingSubsystem::OpenStartupTargetLevel,
            RemainingSeconds,
            false);
    }
}

void UPTLoadingSubsystem::OpenStartupTargetLevel()
{
    if (StartupTargetLevelName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Startup] StartupTargetLevelName is None."));
        FinishLoading();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Startup] Opening target level: %s"), *StartupTargetLevelName.ToString());
    UGameplayStatics::OpenLevel(GetWorld(), StartupTargetLevelName);
}

void UPTLoadingSubsystem::ReleaseRetainedPreloadHandle()
{
    RetainedPreloadHandle.Reset();
}

void UPTLoadingSubsystem::ShowLoadingWidget()
{
    UGameInstance* GameInstance = GetGameInstance();
    APlayerController* PlayerController =
        GameInstance != nullptr ? GameInstance->GetFirstLocalPlayerController() : nullptr;
    if (PlayerController == nullptr)
    {
        return;
    }

    if (LoadingWidgetInstance != nullptr)
    {
        LoadingWidgetInstance->SetLoadingContext(LoadingContext);
        return;
    }

    const UPTUISettings* UISettings = GetDefault<UPTUISettings>();
    TSubclassOf<UPTLoadingWidget> LoadingWidgetClass = UPTLoadingWidget::StaticClass();
    int32 LoadingWidgetZOrder = 10000;

    if (UISettings != nullptr)
    {
        LoadingWidgetZOrder = UISettings->LoadingWidgetZOrder;
        if (!UISettings->LoadingWidgetClass.IsNull())
        {
            LoadingWidgetClass = UISettings->LoadingWidgetClass.LoadSynchronous();
        }
    }

    if (LoadingWidgetClass == nullptr)
    {
        LoadingWidgetClass = UPTLoadingWidget::StaticClass();
    }

    LoadingWidgetInstance = CreateWidget<UPTLoadingWidget>(PlayerController, LoadingWidgetClass);
    if (LoadingWidgetInstance == nullptr)
    {
        return;
    }

    LoadingWidgetInstance->SetLoadingContext(LoadingContext);
    LoadingWidgetInstance->AddToPlayerScreen(LoadingWidgetZOrder);
}

void UPTLoadingSubsystem::HideLoadingWidget()
{
    if (LoadingWidgetInstance == nullptr)
    {
        return;
    }

    LoadingWidgetInstance->RemoveFromParent();
    LoadingWidgetInstance = nullptr;
}

void UPTLoadingSubsystem::CollectStartupPreloadPaths(
    const TArray<TSoftObjectPtr<UObject>>& AdditionalAssetRefs,
    const TArray<TSoftClassPtr<UObject>>& AdditionalClassRefs,
    TSet<FSoftObjectPath>& OutPaths) const
{
    const UPTUISettings* UISettings = GetDefault<UPTUISettings>();
    if (UISettings != nullptr)
    {
        for (const TPair<FName, FPTUILevelEntry>& LevelUIEntry : UISettings->LevelUITable)
        {
            AddSoftPath(LevelUIEntry.Value.WidgetClass.ToSoftObjectPath(), OutPaths);
        }

        AddSoftPath(UISettings->LoadingWidgetClass.ToSoftObjectPath(), OutPaths);

        for (const TSoftObjectPtr<UTexture2D>& BackgroundImage : UISettings->LoadingBackgroundImages)
        {
            AddSoftPath(BackgroundImage.ToSoftObjectPath(), OutPaths);
        }
    }

    for (const TSoftObjectPtr<UObject>& AssetRef : AdditionalAssetRefs)
    {
        AddSoftPath(AssetRef.ToSoftObjectPath(), OutPaths);
    }

    for (const TSoftClassPtr<UObject>& ClassRef : AdditionalClassRefs)
    {
        AddSoftPath(ClassRef.ToSoftObjectPath(), OutPaths);
    }
}

void UPTLoadingSubsystem::CollectDataTableSoftPaths(UDataTable* DataTable, TSet<FSoftObjectPath>& OutPaths) const
{
    if (DataTable == nullptr || DataTable->GetRowStruct() == nullptr)
    {
        return;
    }

    const UScriptStruct* RowStruct = DataTable->GetRowStruct();
    for (const TPair<FName, uint8*>& RowPair : DataTable->GetRowMap())
    {
        const uint8* RowData = RowPair.Value;
        if (RowData == nullptr)
        {
            continue;
        }

        for (TFieldIterator<FProperty> PropertyIt(RowStruct); PropertyIt; ++PropertyIt)
        {
            const FProperty* Property = *PropertyIt;
            const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);
            CollectPropertySoftPaths(Property, ValuePtr, OutPaths);
        }
    }
}

void UPTLoadingSubsystem::CollectPropertySoftPaths(
    const FProperty* Property,
    const void* ValuePtr,
    TSet<FSoftObjectPath>& OutPaths) const
{
    if (Property == nullptr || ValuePtr == nullptr)
    {
        return;
    }

    if (const FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property))
    {
        AddSoftPath(SoftObjectProperty->GetPropertyValue(ValuePtr).ToSoftObjectPath(), OutPaths);
        return;
    }

    if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
    {
        if (StructProperty->Struct == nullptr)
        {
            return;
        }

        for (TFieldIterator<FProperty> PropertyIt(StructProperty->Struct); PropertyIt; ++PropertyIt)
        {
            const FProperty* InnerProperty = *PropertyIt;
            const void* InnerValuePtr = InnerProperty->ContainerPtrToValuePtr<void>(ValuePtr);
            CollectPropertySoftPaths(InnerProperty, InnerValuePtr, OutPaths);
        }
        return;
    }

    if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
    {
        FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
        for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
        {
            CollectPropertySoftPaths(ArrayProperty->Inner, ArrayHelper.GetRawPtr(Index), OutPaths);
        }
        return;
    }

    if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
    {
        FScriptMapHelper MapHelper(MapProperty, ValuePtr);
        for (int32 Index = 0; Index < MapHelper.GetMaxIndex(); ++Index)
        {
            if (!MapHelper.IsValidIndex(Index))
            {
                continue;
            }

            CollectPropertySoftPaths(MapProperty->KeyProp, MapHelper.GetKeyPtr(Index), OutPaths);
            CollectPropertySoftPaths(MapProperty->ValueProp, MapHelper.GetValuePtr(Index), OutPaths);
        }
        return;
    }

    if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
    {
        FScriptSetHelper SetHelper(SetProperty, ValuePtr);
        for (int32 Index = 0; Index < SetHelper.GetMaxIndex(); ++Index)
        {
            if (!SetHelper.IsValidIndex(Index))
            {
                continue;
            }

            CollectPropertySoftPaths(SetProperty->ElementProp, SetHelper.GetElementPtr(Index), OutPaths);
        }
    }
}

void UPTLoadingSubsystem::AddSoftPath(const FSoftObjectPath& Path, TSet<FSoftObjectPath>& OutPaths) const
{
    if (Path.IsValid())
    {
        OutPaths.Add(Path);
    }
}
