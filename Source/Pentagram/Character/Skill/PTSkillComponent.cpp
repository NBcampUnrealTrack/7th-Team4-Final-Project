#include "PTSkillComponent.h"

#include "Character/PTBaseCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

static constexpr int32 MaxSkillSlots = 4;

UPTSkillComponent::UPTSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    // 스킬 슬롯 초기화
    SkillSlots.Init(NAME_None, MaxSkillSlots);
    CooldownTimers.SetNum(MaxSkillSlots);
    bIsCooldown.Init(false, MaxSkillSlots);
}

void UPTSkillComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPTSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (FTimerHandle& Handle : CooldownTimers)
        {
            World->GetTimerManager().ClearTimer(Handle);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void UPTSkillComponent::TryActivateSkill(const FPTSkillActivationRequest& Request)
{
    TryActivateSkillChecked(Request);
}

bool UPTSkillComponent::TryActivateSkillChecked(const FPTSkillActivationRequest& Request)
{
    if (Request.SkillRowName == NAME_None)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->HasAuthority())
    {
        return false;
    }

    const FPTSkillRow* SkillData = FindSkillRowFromRequest(Request);
    if (!SkillData)
    {
        return false;
    }

    const int32 SlotIndex = SkillSlots.IndexOfByKey(Request.SkillRowName);
    if (SlotIndex != INDEX_NONE && bIsCooldown.IsValidIndex(SlotIndex) && bIsCooldown[SlotIndex])
    {
        return false;
    }

    UAnimMontage* Montage = nullptr;
    if (!Request.OverrideMontage.IsNull())
    {
        Montage = Request.OverrideMontage.LoadSynchronous();
    }
    else
    {
        Montage = SkillData->SkillMontage.LoadSynchronous();
    }

    if (!IsValid(Montage))
    {
        return false;
    }

    CurrentSkillID = Request.SkillRowName;

    UNiagaraSystem* Effect = SkillData->SkillEffect.LoadSynchronous();
    USoundBase* Sound = SkillData->SkillSound.LoadSynchronous();

    Multicast_PlaySkillMontageWithOffset(Montage, Effect, Sound, SkillData->SkillOffset);

    if (SkillData->Cooldown <= 0.f)
    {
        return true;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return true;
    }

    if (SlotIndex != INDEX_NONE && bIsCooldown.IsValidIndex(SlotIndex))
    {
        bIsCooldown[SlotIndex] = true;
        World->GetTimerManager().SetTimer(
            CooldownTimers[SlotIndex],
            [this, SlotIndex]() { OnCooldownEnd(SlotIndex); },
            SkillData->Cooldown, false);
    }

    return true;
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
    if (!bIsCooldown.IsValidIndex(SlotIndex))
    {
        return;
    }

    bIsCooldown[SlotIndex] = false;
    UE_LOG(LogTemp, Warning, TEXT("Skill 쿨다운 종료 - 슬롯: %d"), SlotIndex);
}

const FPTSkillRow* UPTSkillComponent::FindSkillRowFromRequest(const FPTSkillActivationRequest& Request) const
{
    UDataTable* DT = Request.SkillDataTable ? Request.SkillDataTable.Get() : SkillDataTable.Get();
    if (!DT)
    {
        return nullptr;
    }

    return DT->FindRow<FPTSkillRow>(Request.SkillRowName, TEXT("FindSkillRowFromRequest"));
}

float UPTSkillComponent::GetCooldownRemaining(int32 SlotIndex) const
{
    if (!bIsCooldown.IsValidIndex(SlotIndex) || !CooldownTimers.IsValidIndex(SlotIndex))
    {
        return 0.f;
    }

    if (!bIsCooldown[SlotIndex]) return 0.f;

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return 0.f;
    }

    return World->GetTimerManager().GetTimerRemaining(CooldownTimers[SlotIndex]);
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

void UPTSkillComponent::Multicast_PlaySkillMontageWithOffset_Implementation(UAnimMontage* Montage, UNiagaraSystem* Effect, USoundBase* Sound, FVector SkillOffset)
{
    if (!IsValid(Montage))
    {
        return;
    }

    APTBaseCharacter* Owner = Cast<APTBaseCharacter>(GetOwner());
    if (!IsValid(Owner))
    {
        return;
    }

    Owner->PlayAnimMontage(Montage);

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorRotation().RotateVector(SkillOffset);
    FRotator SpawnRotation = Owner->GetActorRotation();

    if (Effect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, Effect, SpawnLocation, SpawnRotation);
    }

    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(World, Sound, SpawnLocation);
    }
}
