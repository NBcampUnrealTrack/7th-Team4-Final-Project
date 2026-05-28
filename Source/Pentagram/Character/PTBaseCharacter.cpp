#include "Character/PTBaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/PTBasePlayerState.h"
#include "Player/PTInventoryComponent.h"
#include "Player/PTEquipmentComponent.h"
#include "Item/PTDropItemActorBase.h"


APTBaseCharacter::APTBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    InventoryComponent = CreateDefaultSubobject<UPTInventoryComponent>(TEXT("InventoryComponent")); // 캐릭터의 서브오브젝트로 인벤토리 컴포넌트 생성/장착
    EquipmentComponent = CreateDefaultSubobject<UPTEquipmentComponent>(TEXT("EquipmentComponent")); // 캐릭터의 서브오브젝트로 장비창 컴포넌트 생성/장착
}

float APTBaseCharacter::ApplyDamage(float DamageAmount, AActor* Attacker)
{
    if (!HasAuthority()) return 0.f;

    // 데미지 계산
    float FinalDamage = FMath::Max(DamageAmount - BaseDef, 1.f);
    //HP 감소
    CurrentHP = FMath::Max(CurrentHP - FinalDamage,0.f);

    //PlayerState에 결과 반영 (플레이어만)
    APTBasePlayerState* PS = GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentHP = CurrentHP;
        PS->MaxHP = MaxHP;
    }

    // 죽음
    if (CurrentHP <= 0.f)
    {
        OnDeath();
    }

    return FinalDamage;
}

void APTBaseCharacter::OnDeath()
{
    // 이동 불가
    GetCharacterMovement()->DisableMovement();

    // 콜리전 비활성화(액터끼리 충돌 안함, 나중에 리스폰 시에 활성화 시켜줘야 할 수 있음.)
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //사망 애니메이션 재생은 각 파생 클래스에서 구현해주세요.
}

void APTBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

    // DT에서 캐릭터 스탯을 로드
    if (const FPTCharacterRow* Row = CharacterDataHandle.GetRow<FPTCharacterRow>(TEXT("BeginPlay")))
    {
        MaxHP = Row->MaxHP;
        CurrentHP = Row->MaxHP;
        MaxMP = Row->MaxMP;
        CurrentMP = Row->MaxMP;
        BaseDef = Row->BaseDef;
        BaseAtk = Row->BaseAtk;
        AttackSpeed = Row->AttackSpeed;
        MoveSpeed = Row->MoveSpeed;
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
}

void APTBaseCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTBaseCharacter, CurrentHP);
    DOREPLIFETIME(APTBaseCharacter, MaxHP);
    DOREPLIFETIME(APTBaseCharacter, CurrentMP);
    DOREPLIFETIME(APTBaseCharacter, MaxMP);
    DOREPLIFETIME(APTBaseCharacter, BaseDef);
    DOREPLIFETIME(APTBaseCharacter, BaseAtk);
    DOREPLIFETIME(APTBaseCharacter, AttackSpeed);
    DOREPLIFETIME(APTBaseCharacter, MoveSpeed);
}

void APTBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// =========================================================================
// 입력 연동 및 상호작용 구현
// =========================================================================

void APTBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 마우스 좌클릭 입력을 "LeftClick" 액션 이름과 맵핑
    PlayerInputComponent->BindAction(TEXT("LeftClick"), IE_Pressed, this, &APTBaseCharacter::Input_MouseLeftClick);
}

void APTBaseCharacter::Input_MouseLeftClick()
{
    // 마우스 커서 아래에 아이템이 있는지 검사하기 위해 컨트롤러를 확보
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !InventoryComponent) return;

    FHitResult HitResult;
    // (탑뷰 시점) 마우스 커서가 가리키는 오브젝트 체크
    if (PC->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult))
    {
        APTDropItemActorBase* TargetItem = Cast<APTDropItemActorBase>(HitResult.GetActor());
        if (TargetItem)
        {
            // 탑뷰 전용 보정: Z축 높낮이를 무시하고 평면상 거리만 1미터(100cm) 이내인지 판정
            float Distance2D = FVector::Dist2D(GetActorLocation(), TargetItem->GetActorLocation());

            if (Distance2D <= 100.0f)
            {
                // 인벤토리 컴포넌트에 드롭 아이템 데이터 전달 및 수량 1개 추가
                if (InventoryComponent->TryAddItem(TargetItem->GetItemData(), 1))
                {
                    UE_LOG(LogTemp, Log, TEXT("[아이템 획득] 평면 거리 %.1fcm에서 %s을(를) 획득!"),
                        Distance2D, *TargetItem->GetItemData().Item_Name.ToString());

                    // 바닥의 액터 제거
                    TargetItem->Destroy();
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("아이템과 거리가 너무 멉니다. (현재 거리: %.1fcm)"), Distance2D);
            }
        }
    }
}
