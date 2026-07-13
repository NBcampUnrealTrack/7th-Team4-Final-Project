#include "PTPlayerSkillComponent.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTEquipmentComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

void UPTPlayerSkillComponent::TryActivateSkill(const FPTSkillActivationRequest& Request)
{
    AActor* OwnerActor = GetOwner();
    if (!IsValid(OwnerActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Skill] TryActivateSkill 실패 - Owner 없음"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Skill] TryActivateSkill 호출 - SkillRowName: %s"),
        *Request.SkillRowName.ToString());

    if (!OwnerActor->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 권한 없음 - 서버가 아님"));
        return;
    }

    // DT에서 스킬 데이터 조회
    const FPTSkillRow* SkillData = FindSkillRowFromRequest(Request);
    if (!SkillData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Skill] 실패 - 스킬 데이터 없음: %s"),
            *Request.SkillRowName.ToString());
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Skill 데이터 조회 성공 - MP소모: %.1f, 쿨다운: %.1f"), SkillData->MPCost, SkillData->Cooldown);

    // 슬롯 인덱스 찾기
    int32 SlotIndex = SkillSlots.IndexOfByKey(Request.SkillRowName);
    if (SlotIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 슬롯에 등록되지 않은 스킬"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Skill 슬롯 인덱스: %d"), SlotIndex);

    // 쿨다운 체크
    if (bIsCooldown[SlotIndex])
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 쿨다운 중 - 남은 시간: %.1f초"), GetCooldownRemaining(SlotIndex));
        return;
    }

    // 스킬 습득 여부
    if (!IsSkillLearned(Request.SkillRowName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 미습득 - %s"), *Request.SkillRowName.ToString());
        return;
    }

    // 무기/레벨 제한
    FText Reason;
    if (!CanUseSkill(*SkillData, Reason))
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 사용 제한 - %s"), *Reason.ToString());
        return;
    }

    // MP 체크 및 차감
    APTBaseCharacter* Owner = Cast<APTBaseCharacter>(GetOwner());
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill Owner 실패"));
        return;
    }

    // 전투 진입 전환
    if (APTPlayerCharacter* GatePC = Cast<APTPlayerCharacter>(Owner))
    {
        if (!GatePC->bIsInCombat || GatePC->bIsTransitioningToCombat)
        {
            GatePC->EnterCombat();
            bPendingSkill = true;
            PendingSkillRequest = Request;
            UE_LOG(LogTemp, Warning, TEXT("Skill 전투 전환 대기 - %s"), *Request.SkillRowName.ToString());
            return;
        }
    }

    if (Owner->CurrentMP < SkillData->MPCost)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill MP 부족 - 현재: %.1f, 필요: %.1f"), Owner->CurrentMP, SkillData->MPCost);
        return;
    }
    Owner->CurrentMP -= SkillData->MPCost;
    UE_LOG(LogTemp, Warning, TEXT("Skill 스킬 발동 성공 - 남은 MP: %.1f"), Owner->CurrentMP);

    // PlayerState MP 반영
    APTBasePlayerState* PS = Owner->GetPlayerState<APTBasePlayerState>();
    if (PS)
    {
        PS->CurrentMP = Owner->CurrentMP;
    }

    CurrentSkillID = Request.SkillRowName;

    AimDirection   = Request.AimDirection;
    TargetLocation = Request.TargetLocation;
    TargetActor    = Request.TargetActor;

    if (!Request.AimDirection.IsNearlyZero())
        Owner->SetActorRotation(Request.AimDirection.Rotation());

    // 쿨다운 시작
    UWorld* World = GetWorld();
    if (!World) return;

    if (SkillData->Cooldown > 0.f)
    {
        bIsCooldown[SlotIndex] = true;

        int32 CapturedSlotIndex = SlotIndex;
        World->GetTimerManager().SetTimer(
            CooldownTimers[SlotIndex],
            [this, CapturedSlotIndex]() { OnCooldownEnd(CapturedSlotIndex); },
            SkillData->Cooldown,
            false
        );

        // 발동 성공 시점에 소유 클라로 "쿨다운 시작" 통지
        Client_NotifyCooldownStarted(SlotIndex, SkillData->Cooldown);
    }

    if (UAnimMontage* Montage = SkillData->SkillMontage.LoadSynchronous())
    {
        APTPlayerCharacter* PlayerChar = Cast<APTPlayerCharacter>(Owner);
        if (PlayerChar)
        {
            bIsAttacking = false;
            bCanCombo    = false;
            ComboIndex   = 0;
            Owner->StopAnimMontage();
        }

        // 관통 스킬이면 충돌 무시
        if (UAnimInstance* AnimInst = Owner->GetMesh()->GetAnimInstance())
        {
            AnimInst->OnMontageEnded.AddUniqueDynamic(
                this, &UPTPlayerSkillComponent::OnSkillMontageEnded);
        }

        if (SkillData->bPenetrate)
        {
            Multicast_SetPenetration(true);
        }

        if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(Owner))
        {
            PC->bIsUsingSkill = true;
        }

        // 이펙트/사운드 에셋 로드 (서버에서 한 번만 로드 후 Multicast로 전달)
        UNiagaraSystem* Effect = SkillData->SkillEffect.LoadSynchronous();
        USoundBase*     Sound  = SkillData->SkillSound.LoadSynchronous();

        const bool bIsAreaSkill =
    SkillData->IndicatorShape == ESkillIndicatorShape::Circle ||
    SkillData->IndicatorShape == ESkillIndicatorShape::SelfCircle;

        if (SkillData->TargetingMode == ESkillTargetingMode::Targeted)
        {
            // 목표 위치에 vfx, 그 대상에게 데미지. 목표타겟 없으면 캐릭터 위치에서 헛방
            const FVector Center = IsValid(TargetActor)
                ? TargetActor->GetActorLocation()
                : Owner->GetActorLocation();

            Multicast_PlaySkillMontageAtLocation(Montage, nullptr, Sound, Center, Request.SkillRowName);
        }
        else if (bIsAreaSkill)
        {
            const FVector Center = (SkillData->IndicatorShape == ESkillIndicatorShape::SelfCircle)
                ? Owner->GetActorLocation()
                : FVector(TargetLocation);

            Multicast_PlaySkillMontageAtLocation(Montage, nullptr, Sound, Center, Request.SkillRowName);
        }
        else
        {
            Multicast_PlaySkillMontageWithOffset(Montage, nullptr, Sound, SkillData->SkillOffset, Request.SkillRowName);
        }

        if (SkillData->BuffDuration > 0.f && SkillData->AtkBuffMultiplier > 0.f)
        {
            if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(Owner))
            {
                PC->ApplyBuff(SkillData->AtkBuffMultiplier, SkillData->BuffDuration);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 몽타주 없음"));
    }

    if (APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(Owner))
    {
        PC->EnterCombat();
    }
}

void UPTPlayerSkillComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UPTPlayerSkillComponent, LearnedSkills, COND_OwnerOnly);
}

void UPTPlayerSkillComponent::TryDodge()
{
    UE_LOG(LogTemp, Warning, TEXT("TryDodge 진입"));

    const FPTSkillRow* DodgeData = GetSkillData(DodgeSkillID);
    if (!DodgeData)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryDodge - DT에 Dodge 데이터 없음. ID: %s"), *DodgeSkillID.ToString());
        return;
    }

    UAnimMontage* Montage = DodgeData->SkillMontage.LoadSynchronous();
    if (!Montage) return;

    // 로컬 클라이언트 즉시 몽타주 재생
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    // 로컬에서 닷지 상태 체크
    if (PC->IsLocallyControlled())
    {
        PC->PlayAnimMontage(Montage);
        PC->bIsDodging = true;
    }

    // 서버 RPC 호출
    Server_Dodge();

    // 로컬 쿨다운 시작 및 UI 통지
    bIsCooldown[4] = true;
    GetWorld()->GetTimerManager().SetTimer(
        CooldownTimers[4],
        [this]() { OnCooldownEnd(4); },
        DodgeData->Cooldown,
        false
        );

    Client_NotifyCooldownStarted(4, DodgeData->Cooldown);
}

void UPTPlayerSkillComponent::TryBasicAttack()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;
    if (PC->bIsDodging || PC->bIsUsingSkill) return;

    if (bIsAttacking && !bCanCombo) return;

    if (!PC->bIsInCombat || PC->bIsTransitioningToCombat)
    {
        PC->EnterCombat();
        bPendingBasicAttack = true;
        return;
    }

    ExecuteBasicAttack();
}

void UPTPlayerSkillComponent::PlayPendingAction()
{
    if (bPendingBasicAttack)
    {
        bPendingBasicAttack = false;
        ExecuteBasicAttack();
        return;
    }

    if (bPendingSkill)
    {
        bPendingSkill = false;
        FPTSkillActivationRequest Request = PendingSkillRequest;
        TryActivateSkill(Request);
    }
}

void UPTPlayerSkillComponent::ExecuteBasicAttack()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    const FPTSkillRow* SkillData = GetSkillData(BasicAttackSkillID);
    if (!SkillData) return;

    const TArray<TSoftObjectPtr<UAnimMontage>>& ComboArray = SkillData->GetComboMontageForWeapon(PC->CurrentWeaponType);
    if (ComboArray.Num() == 0) return;

    if (!ComboArray.IsValidIndex(ComboIndex))
    {
        ComboIndex = 0;
    }

    UAnimMontage* Montage = ComboArray[ComboIndex].LoadSynchronous();
    if (!Montage) return;

    bIsAttacking = true;
    bCanCombo = false;

    if (PC->IsLocallyControlled())
    {
        PC->PlayAnimMontage(Montage);
    }

    Server_BasicAttack(ComboIndex);
    ComboIndex++;
}

void UPTPlayerSkillComponent::ApplyRadialDamageAtLocation(const FPTSkillRow& Row, const FVector& Center)
{
    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(GetOwner());
    if (!Owner || !Owner->HasAuthority()) return;

    UWorld* World = GetWorld();
    if (!IsValid(World)) return;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> OverlapActors;
    UKismetSystemLibrary::SphereOverlapActors(
        World, Center, Row.SkillRadius, ObjectTypes, nullptr,
        TArray<AActor*>{ Owner }, OverlapActors);

    const float FinalDamage = Owner->GetTotalAttack() * Row.DamageMultiplier;

    for (AActor* HitActor : OverlapActors)
    {
        APTBaseCharacter* Target = Cast<APTBaseCharacter>(HitActor);
        if (!Target || Cast<APTPlayerCharacter>(Target)) continue;

        FPTHitInfo HitInfo = Row.MakeHitInfo(Owner);
        FVector Dir = Target->GetActorLocation() - Center; Dir.Z = 0.f;
        HitInfo.HitDirection = Dir.GetSafeNormal();   // 중심에서 바깥으로 넉백

        Target->ApplyDamageWithHit(FinalDamage, Owner, HitInfo);

        UE_LOG(LogTemp, Log, TEXT("AoE %s 명중 / 데미지 %.1f"), *Target->GetName(), FinalDamage);
    }

    if (USoundBase* HitSound = Row.SkillHitSound.LoadSynchronous())
        Multicast_PlayHitSound(HitSound, Center);
}

void UPTPlayerSkillComponent::ApplyTargetedDamage(const FPTSkillRow& Row, AActor* Target)
{
    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(GetOwner());
    if (!Owner || !Owner->HasAuthority()) return;

    APTBaseCharacter* Victim = Cast<APTBaseCharacter>(Target);
    if (!Victim || Cast<APTPlayerCharacter>(Victim)) return;

    const float FinalDamage = Owner->GetTotalAttack() * Row.DamageMultiplier;

    FPTHitInfo HitInfo = Row.MakeHitInfo(Owner);
    FVector Dir = Victim->GetActorLocation() - Owner->GetActorLocation(); Dir.Z = 0.f;
    HitInfo.HitDirection = Dir.GetSafeNormal();

    Victim->ApplyDamageWithHit(FinalDamage, Owner, HitInfo);

    UE_LOG(LogTemp, Log, TEXT("Targeted %s 명중 / 데미지 %.1f"), *Victim->GetName(), FinalDamage);

    if (USoundBase* HitSound = Row.SkillHitSound.LoadSynchronous())
        Multicast_PlayHitSound(HitSound, Victim->GetActorLocation());
}

void UPTPlayerSkillComponent::Server_Dodge_Implementation()
{
    const FPTSkillRow* DodgeData = GetSkillData(DodgeSkillID);
    if (!DodgeData) return;

    UAnimMontage* Montage = DodgeData->SkillMontage.LoadSynchronous();
    if (!Montage) return;

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    PC->bIsDodging = true;

    FVector LaunchDir = PC->GetActorForwardVector();
    LaunchDir.Z = 0.f;

    if (UAnimInstance* AnimInst = PC->GetMesh()->GetAnimInstance())
    {
        AnimInst->OnMontageEnded.AddDynamic(this, &UPTPlayerSkillComponent::OnDodgeMontageEnded);
    }

    Multicast_PlayDodgeMontage(Montage);
}

void UPTPlayerSkillComponent::Server_SetInvincible_Implementation(bool bInvincible)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    PC->bIsInvincible = bInvincible;
    UE_LOG(LogTemp, Warning, TEXT("무적 상태 변경: %s"), bInvincible ? TEXT("ON") : TEXT("OFF"));
}

void UPTPlayerSkillComponent::Multicast_PlayDodgeMontage_Implementation(UAnimMontage* Montage)
{
    if (!Montage) return;

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    // 로컬 클라이언트는 TryDodge()에서 이미 재생
    if (PC->IsLocallyControlled()) return;

    PC->PlayAnimMontage(Montage);
}

void UPTPlayerSkillComponent::Multicast_OnDodgeEnded_Implementation()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    PC->bIsDodging = false;
}

void UPTPlayerSkillComponent::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    const FPTSkillRow* DodgeData = GetSkillData(DodgeSkillID);
    if (!DodgeData) return;

    UAnimMontage* DodgeMontage = DodgeData->SkillMontage.LoadSynchronous();
    if (DodgeMontage != Montage) return;

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    UAnimInstance* AnimInst = PC->GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->OnMontageEnded.RemoveDynamic(this, &UPTPlayerSkillComponent::OnDodgeMontageEnded);
    }

    Multicast_OnDodgeEnded();
}

void UPTPlayerSkillComponent::OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    const FPTSkillRow* SkillData = GetSkillData(CurrentSkillID);
    if (!SkillData) return;

    UAnimMontage* SkillMontage = SkillData->SkillMontage.LoadSynchronous();
    if (SkillMontage != Montage) return;

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    if (UAnimInstance* AnimInst = PC->GetMesh()->GetAnimInstance())
    {
        AnimInst->OnMontageEnded.RemoveDynamic(this, &UPTPlayerSkillComponent::OnSkillMontageEnded);
    }

    PC->bIsUsingSkill = false;

    Multicast_SetPenetration(false);
}

void UPTPlayerSkillComponent::Server_BasicAttack_Implementation(int32 InComboIndex)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (PC)
    {
        PC->EnterCombat();
    }

    Multicast_PlayBasicAttackMontage(InComboIndex);
}

void UPTPlayerSkillComponent::Multicast_PlayBasicAttackMontage_Implementation(int32 InComboIndex)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    if (PC->IsLocallyControlled()) return;

    const FPTSkillRow* SkillData = GetSkillData(BasicAttackSkillID);
    if (!SkillData) return;

    const TArray<TSoftObjectPtr<UAnimMontage>>& ComboArray = SkillData->GetComboMontageForWeapon(PC->CurrentWeaponType);

    if (!ComboArray.IsValidIndex(InComboIndex)) return;

    if (UAnimMontage* Montage = ComboArray[InComboIndex].LoadSynchronous())
    {
        PC->PlayAnimMontage(Montage);
    }
}

void UPTPlayerSkillComponent::Server_StopBasicAttack_Implementation()
{
    Multicast_StopBasicAttack();
}

void UPTPlayerSkillComponent::Multicast_StopBasicAttack_Implementation()
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    PC->StopAnimMontage();
    bIsAttacking = false;
    bCanCombo = false;
    ComboIndex = 0;
}

void UPTPlayerSkillComponent::Multicast_SetPenetration_Implementation(bool bEnable)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    UCapsuleComponent* Capsule = PC->GetCapsuleComponent();
    if (!Capsule) return;

    if (bEnable)
    {
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }
    else
    {
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }
}

void UPTPlayerSkillComponent::Multicast_LaunchForSkill_Implementation(FVector Velocity)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;

    Character->LaunchCharacter(Velocity, true, false);
}


void UPTPlayerSkillComponent::Multicast_StopMovementForSkill_Implementation()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;

    if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
    }
}

void UPTPlayerSkillComponent::TryActivateSkillBySlot(int32 SlotIndex)
{
    if (!SkillSlots.IsValidIndex(SlotIndex)) return;

    const FName SkillID = SkillSlots[SlotIndex];
    if (SkillID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill 슬롯 %d 비어있음"), SlotIndex)
        return;
    }

    FPTSkillActivationRequest Request;
    Request.SkillRowName = SkillID;
    Request.SkillDataTable = SkillDataTable;
    TryActivateSkill(Request);
}

void UPTPlayerSkillComponent::OnRep_LearnedSkills()
{
    OnSkillLearned.Broadcast(NAME_None);
}

bool UPTPlayerSkillComponent::IsSkillLearned(FName SkillID) const
{
    return SkillID != NAME_None && LearnedSkills.Contains(SkillID);
}

bool UPTPlayerSkillComponent::LearnSkill(FName SkillID)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority() || SkillID.IsNone()) return false;
    if (LearnedSkills.Contains(SkillID)) return false;

    const FPTSkillRow* Row = GetSkillData(SkillID);
    if (!Row) return false;

    if (GetOwnerLevel() < Row->RequiredLevel) return false; // 레벨 제한

    LearnedSkills.Add(SkillID);
    OnSkillLearned.Broadcast(SkillID);   // 리슨서버 호스트 UI 갱신
    return true;
}

bool UPTPlayerSkillComponent::RestoreSkillProgress(
    const TArray<FName>& InLearnedSkills,
    const TArray<FName>& InSkillSlots)
{
    AActor* Owner = GetOwner();
    if (Owner == nullptr || !Owner->HasAuthority())
    {
        return false;
    }

    TArray<FName> RestoredLearnedSkills;
    for (FName SkillID : InLearnedSkills)
    {
        if (SkillID.IsNone() || RestoredLearnedSkills.Contains(SkillID) || GetSkillData(SkillID) == nullptr)
        {
            continue;
        }

        RestoredLearnedSkills.Add(SkillID);
    }
    LearnedSkills = MoveTemp(RestoredLearnedSkills);

    const int32 SlotCount = SkillSlots.IsEmpty() ? 5 : SkillSlots.Num();
    TArray<FName> RestoredSkillSlots;
    RestoredSkillSlots.Init(NAME_None, SlotCount);
    const int32 CopyCount = FMath::Min(InSkillSlots.Num(), SlotCount);
    for (int32 Index = 0; Index < CopyCount; ++Index)
    {
        const FName SkillID = InSkillSlots[Index];
        if (!SkillID.IsNone() && GetSkillData(SkillID) != nullptr)
        {
            RestoredSkillSlots[Index] = SkillID;
        }
    }
    SkillSlots = MoveTemp(RestoredSkillSlots);

    OnSkillLearned.Broadcast(NAME_None);
    for (int32 Index = 0; Index < SkillSlots.Num(); ++Index)
    {
        Client_NotifySkillSlotAssigned(Index, SkillSlots[Index]);
    }

    Owner->ForceNetUpdate();
    return true;
}

bool UPTPlayerSkillComponent::CanUseSkill(const FPTSkillRow& Row, FText& OutReason) const
{
    if (GetOwnerLevel() < Row.RequiredLevel)
    {
        OutReason = FText::FromString(TEXT("레벨이 부족합니다."));
        return false;
    }
    if (const APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner()))
    {
        if (!Row.IsWeaponAllowed(PC->CurrentWeaponType))
        {
            OutReason = FText::FromString(TEXT("현재 무기로는 사용할 수 없습니다."));
            return false;
        }
    }
    return true;
}

void UPTPlayerSkillComponent::OnCooldownEnd(int32 SlotIndex)
{
    Super::OnCooldownEnd(SlotIndex);

    // UI 쿨다운 종료 델리게이트 발행
    OnSkillCooldownEnd.Broadcast(SlotIndex);
}

int32 UPTPlayerSkillComponent::GetOwnerLevel() const
{
    if (const APawn* P = Cast<APawn>(GetOwner()))
    {
        if (const APTBasePlayerState* PS = P->GetPlayerState<APTBasePlayerState>())
        {
            // TODO: PlayerState의 실제 레벨 필드/게터에 맞춰 교체
            // return PS->CurrentLevel;
        }
    }
    return 1;
}

void UPTPlayerSkillComponent::Client_NotifyCooldownStarted_Implementation(int32 SlotIndex, float Duration)
{
    OnSkillCooldownStart.Broadcast(SlotIndex, Duration);
}

void UPTPlayerSkillComponent::Client_NotifySkillSlotAssigned_Implementation(int32 SlotIndex, FName SkillID)
{
    OnSkillSlotAssigned.Broadcast(SlotIndex, SkillID);
}

void UPTPlayerSkillComponent::TriggerCachedAoEDamage()
{
    if (const FPTSkillRow* Row = GetSkillData(CurrentSkillID))
    {
        const FVector Center = (Row->IndicatorShape == ESkillIndicatorShape::SelfCircle)
            ? GetOwner()->GetActorLocation() : FVector(TargetLocation);
        ApplyRadialDamageAtLocation(*Row, Center);
    }
}
