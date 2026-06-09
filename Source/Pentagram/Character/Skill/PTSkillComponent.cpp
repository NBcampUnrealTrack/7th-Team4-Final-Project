#include "PTSkillComponent.h"

#include "Character/PTBaseCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UPTSkillComponent::UPTSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    // 스킬 슬롯 초기화
    SkillSlots.Init(NAME_None, 4);
    CooldownTimers.SetNum(4);
    bIsCooldown.Init(false, 4);
}

void UPTSkillComponent::BeginPlay()
{
    Super::BeginPlay();
}

FPTSkillRow* UPTSkillComponent::GetSkillData(FName SkillID) const
{
    if (!SkillDataTable) return nullptr;
    return SkillDataTable->FindRow<FPTSkillRow>(SkillID, TEXT("GetSkillData"));
}

void UPTSkillComponent::AssignSkillToSlot(FName SkillID, int32 SlotIndex)
{
    if (!SkillSlots.IsValidIndex(SlotIndex)) return;
    SkillSlots[SlotIndex] = SkillID;
}

FName UPTSkillComponent::GetSkillAtSlot(int32 SlotIndex) const
{
    if (!SkillSlots.IsValidIndex(SlotIndex)) return NAME_None;
    return SkillSlots[SlotIndex];
}

void UPTSkillComponent::OnCooldownEnd(int32 SlotIndex)
{
    bIsCooldown[SlotIndex] = false;
    UE_LOG(LogTemp, Warning, TEXT("Skill 쿨다운 종료 - 슬롯: %d"), SlotIndex);
}

float UPTSkillComponent::GetCooldownRemaining(int32 SlotIndex) const
{
    if (!bIsCooldown[SlotIndex]) return 0.f;
    return GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimers[SlotIndex]);
}

void UPTSkillComponent::Multicast_PlaySkillMontage_Implementation(UAnimMontage* Montage, UNiagaraSystem* Effect, USoundBase* Sound)
{
    if (!Montage) return;

    APTBaseCharacter* Owner = Cast<APTBaseCharacter>(GetOwner());
    if (!Owner) return;

    // 몽타주 재생
    Owner->PlayAnimMontage(Montage);

    // 스킬 발동 위치 계산
    FVector SkillOffset = FVector::ZeroVector;
    const FPTSkillRow* SkillData = GetSkillData(CurrentSkillID);
    if (SkillData)
    {
        SkillOffset = SkillData->SkillOffset;
    }

    FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorRotation().RotateVector(SkillOffset);
    FRotator SpawnRotation = Owner->GetActorRotation();

    // 나이아가라 이펙트
    if (Effect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, SpawnLocation, SpawnRotation);
    }

    // 사운드
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, SpawnLocation);
    }
}
