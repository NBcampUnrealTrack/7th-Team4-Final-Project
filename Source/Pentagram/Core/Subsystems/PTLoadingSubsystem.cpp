#include "Core/Subsystems/PTLoadingSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "UI/Setting/PTUISettings.h"
#include "UI/Widget/Loading/PTLoadingWidget.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UnrealType.h"

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
        ActivePreloadHandle->CancelHandle();
        ActivePreloadHandle.Reset();
    }

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

float UPTLoadingSubsystem::GetLoadingProgress() const
{
    if (ActivePreloadHandle.IsValid())
    {
        return ActivePreloadHandle->GetProgress();
    }

    return bIsLoading ? 0.f : 1.f;
}

void UPTLoadingSubsystem::BeginLoading(FName InLoadingContext)
{
    LoadingContext = InLoadingContext;
    if (bIsLoading)
    {
        if (LoadingWidgetInstance != nullptr)
        {
            LoadingWidgetInstance->SetLoadingContext(LoadingContext);
        }
        return;
    }

    bIsLoading = true;
    ShowLoadingWidget();
    OnLoadingStarted.Broadcast(LoadingContext);
}

void UPTLoadingSubsystem::FinishLoading()
{
    if (!bIsLoading)
    {
        return;
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
    }
}

void UPTLoadingSubsystem::AddSoftPath(const FSoftObjectPath& Path, TSet<FSoftObjectPath>& OutPaths) const
{
    if (Path.IsValid())
    {
        OutPaths.Add(Path);
    }
}
