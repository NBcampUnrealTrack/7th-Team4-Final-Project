
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
