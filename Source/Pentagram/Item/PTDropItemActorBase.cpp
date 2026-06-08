
#include "PTDropItemActorBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h" 


// Sets default values
APTDropItemActorBase::APTDropItemActorBase()
{ 
	PrimaryActorTick.bCanEverTick = false;

    // 충돌 체 구축 (기획서 상 1미터 이내 접근을 위함, 반경 100cm 설정)
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(100.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));

    // 메시 컴포넌트 구축
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시는 충돌 무시
}


void APTDropItemActorBase::BeginPlay()
{
	Super::BeginPlay();
    InitializeItemData(); 
}


void APTDropItemActorBase::InitializeItemData()
{
    if (ItemRowHandle.DataTable != nullptr && !ItemRowHandle.RowName.IsNone())
    {
        FItemData* Data = ItemRowHandle.DataTable->FindRow<FItemData>(ItemRowHandle.RowName, TEXT("ItemInit"));
        if (Data)
        {
            InstanceItemData = *Data;
            UE_LOG(LogTemp, Warning, TEXT("아이템 로드 완료: %s (등급: %d)"), *InstanceItemData.Item_Name.ToString(), (int32)InstanceItemData.Item_Grade);
        }
    }
}
