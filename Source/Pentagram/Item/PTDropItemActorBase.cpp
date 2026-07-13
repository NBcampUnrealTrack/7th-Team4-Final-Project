
#include "PTDropItemActorBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h" 
#include "Net/UnrealNetwork.h"


// Sets default values
APTDropItemActorBase::APTDropItemActorBase()
{ 
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(100.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));

    // 메시 컴포넌트 구축
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);

    // 마우스 클릭(Query)은 작동, 물리 연산(Physics)은 안함.
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 캐릭터나 다른 동적 액터가 지나갈 땐 스무스하게 통과되도록 무시
    ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

    // [중요] 마우스 레이트레이싱 채널만 Block으로 설정
    ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
}


void APTDropItemActorBase::BeginPlay()
{
	Super::BeginPlay();
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

    InstanceItemData = InItemData;
    DroppedQuantity = InQuantity;
    bPickupClaimed = false;
    if (InItemData.ItemMeshAsset.IsNull() && bHadTemplateItem && TemplateItemType != InItemData.Item_Type)
    {
        ItemMesh->SetStaticMesh(nullptr);
    }
    else
    {
        ApplyItemVisual();
    }
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
    if (ItemMesh == nullptr || InstanceItemData.ItemMeshAsset.IsNull())
    {
        return;
    }

    if (UStaticMesh* StaticMesh = InstanceItemData.ItemMeshAsset.LoadSynchronous())
    {
        ItemMesh->SetStaticMesh(StaticMesh);
    }
}


void APTDropItemActorBase::InitializeItemData()
{
    if (ItemRowHandle.DataTable != nullptr && !ItemRowHandle.RowName.IsNone())
    {
        FItemData* Data = ItemRowHandle.DataTable->FindRow<FItemData>(ItemRowHandle.RowName, TEXT("ItemInit"));
        if (Data)
        {
            InstanceItemData = *Data;
            DroppedQuantity = 1;
            ApplyItemVisual();
            UE_LOG(LogTemp, Warning, TEXT("아이템 로드 완료: %s (등급: %d)"), *InstanceItemData.Item_Name.ToString(), (int32)InstanceItemData.Item_Grade);
        }
    }
}
