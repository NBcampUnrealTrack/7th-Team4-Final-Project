#include "Character/PTBaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/PTBasePlayerState.h"

APTBaseCharacter::APTBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
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
        PS->CurrentHP = MaxHP;
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
        BaseDef = Row->BaseDef;
        BaseAtk = Row->BaseAtk;
        AttackSpeed = Row->AttackSpeed;
        MoveSpeed = Row->MoveSpeed;
    }

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void APTBaseCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APTBaseCharacter, CurrentHP);
    DOREPLIFETIME(APTBaseCharacter, MaxHP);
    DOREPLIFETIME(APTBaseCharacter, BaseDef);
    DOREPLIFETIME(APTBaseCharacter, BaseAtk);
    DOREPLIFETIME(APTBaseCharacter, AttackSpeed);
    DOREPLIFETIME(APTBaseCharacter, MoveSpeed);
}

void APTBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void APTBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

