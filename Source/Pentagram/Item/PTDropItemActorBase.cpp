
#include "PTDropItemActorBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h" 
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/Item/PTDropItemNameWidget.h"


// Sets default values
APTDropItemActorBase::APTDropItemActorBase()
{ 
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(100.0f);

    // 메시 컴포넌트 구축
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);

    ItemNameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemNameWidgetComponent"));
    ItemNameWidgetComponent->SetupAttachment(RootComponent);
    ItemNameWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    ItemNameWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    ItemNameWidgetComponent->SetDrawAtDesiredSize(true);
    ItemNameWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
    ItemNameWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ItemNameWidgetComponent->SetGenerateOverlapEvents(false);
    ItemNameWidgetComponent->SetWindowFocusable(false);
    ItemNameWidgetComponent->SetWidgetClass(UPTDropItemNameWidget::StaticClass());
    ItemNameWidgetComponent->SetVisibility(false);

    ConfigureInteractionCollision();
}


void APTDropItemActorBase::BeginPlay()
{
	Super::BeginPlay();
    // 파생 BP에서 컴포넌트 충돌 설정을 덮어썼더라도 클릭 프록시 규칙을 보장합니다.
    ConfigureInteractionCollision();

    if (InstanceItemData.Item_ID.IsNone())
    {
        InitializeItemData();
    }
    else
    {
        ApplyItemVisual();
    }
}

void APTDropItemActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTDropItemActorBase, InstanceItemData);
    DOREPLIFETIME(APTDropItemActorBase, DroppedQuantity);
}

void APTDropItemActorBase::InitializeDroppedItem(const FItemData& InItemData, int32 InQuantity)
{
    if (!HasAuthority() || InItemData.Item_ID.IsNone() || InQuantity <= 0)
    {
        return;
    }

    const bool bHadTemplateItem = !InstanceItemData.Item_ID.IsNone();
    const EItemType TemplateItemType = InstanceItemData.Item_Type;
    const FVector TemplateMeshScale = ItemMesh != nullptr
        ? ItemMesh->GetRelativeScale3D()
        : FVector::OneVector;

    InstanceItemData = InItemData;
    if (InstanceItemData.DropMeshRelativeScale.Equals(FVector::OneVector) &&
        !TemplateMeshScale.Equals(FVector::OneVector))
    {
        // 새 필드가 없던 기존 저장 아이템은 현재 드랍 BP에 설정된 크기를 사용합니다.
        InstanceItemData.DropMeshRelativeScale = TemplateMeshScale;
    }
    DroppedQuantity = InQuantity;
    bPickupClaimed = false;
    if (InItemData.ItemMeshAsset.IsNull() && bHadTemplateItem && TemplateItemType != InItemData.Item_Type)
    {
        ItemMesh->SetStaticMesh(nullptr);
    }
    ApplyItemVisual();
    ForceNetUpdate();
}

bool APTDropItemActorBase::TryClaimPickup()
{
    if (!HasAuthority() || bPickupClaimed || IsActorBeingDestroyed())
    {
        return false;
    }

    bPickupClaimed = true;
    return true;
}

void APTDropItemActorBase::ReleasePickupClaim()
{
    if (HasAuthority() && !IsActorBeingDestroyed())
    {
        bPickupClaimed = false;
    }
}

void APTDropItemActorBase::OnRep_InstanceItemData()
{
    ApplyItemVisual();
}

void APTDropItemActorBase::ApplyItemVisual()
{
    if (ItemMesh != nullptr)
    {
        if (!InstanceItemData.ItemMeshAsset.IsNull())
        {
            if (UStaticMesh* StaticMesh = InstanceItemData.ItemMeshAsset.LoadSynchronous())
            {
                ItemMesh->SetStaticMesh(StaticMesh);
            }
        }

        ItemMesh->SetRelativeScale3D(InstanceItemData.DropMeshRelativeScale);
    }

    RefreshItemNameWidget();
}

void APTDropItemActorBase::RefreshItemNameWidget()
{
    if (ItemNameWidgetComponent == nullptr)
    {
        return;
    }

    const bool bHasItemName = !InstanceItemData.Item_Name.IsEmpty();
    ItemNameWidgetComponent->SetVisibility(bHasItemName);
    if (!bHasItemName)
    {
        return;
    }

    ItemNameWidgetComponent->InitWidget();
    UPTDropItemNameWidget* ItemNameWidget =
        Cast<UPTDropItemNameWidget>(ItemNameWidgetComponent->GetUserWidgetObject());
    if (ItemNameWidget != nullptr)
    {
        ItemNameWidget->SetItemName(InstanceItemData.Item_Name);
    }
}

void APTDropItemActorBase::ConfigureInteractionCollision()
{
    if (CollisionSphere != nullptr)
    {
        // 메시의 Simple Collision 유무와 무관하게 구형 클릭 프록시가 선택됩니다.
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
        CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
        CollisionSphere->SetGenerateOverlapEvents(false);
    }

    if (ItemMesh != nullptr)
    {
        ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        ItemMesh->SetGenerateOverlapEvents(false);
    }
}


void APTDropItemActorBase::InitializeItemData()
{
    if (ItemRowHandle.DataTable != nullptr && !ItemRowHandle.RowName.IsNone())
    {
        FItemData* Data = ItemRowHandle.DataTable->FindRow<FItemData>(ItemRowHandle.RowName, TEXT("ItemInit"));
        if (Data)
        {
            const FVector BlueprintMeshScale = ItemMesh != nullptr
                ? ItemMesh->GetRelativeScale3D()
                : FVector::OneVector;
            InstanceItemData = *Data;
            InstanceItemData.DropMeshRelativeScale = BlueprintMeshScale;
            DroppedQuantity = 1;
            ApplyItemVisual();
            UE_LOG(LogTemp, Warning, TEXT("아이템 로드 완료: %s (등급: %d)"), *InstanceItemData.Item_Name.ToString(), (int32)InstanceItemData.Item_Grade);
        }
    }
}
