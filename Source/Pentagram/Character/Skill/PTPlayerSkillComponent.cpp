#include "PTPlayerSkillComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTEquipmentComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraComponent.h"
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

    // 이미 스킬/채널 사용 중이면 거부 (연타·중복 방지)
    if (const APTPlayerCharacter* GuardPC = Cast<APTPlayerCharacter>(OwnerActor))
    {
        if (GuardPC->bIsUsingSkill || bIsChanneling)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Skill] 이미 스킬 사용 중 - 무시"));
            return;
        }
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

    const APTBasePlayerState* PS = PC->GetPlayerState<APTBasePlayerState>();
    if (PS && PS->CurrentMP < DodgeData->MPCost)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dodge MP 부족(클라) - %.1f/%.1f"),
            PS->CurrentMP, DodgeData->MPCost);
        return;   // 몽타주/쿨다운 시작 전에 차단
    }

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

void UPTPlayerSkillComponent::RefreshChannelProtection()
{
    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(GetOwner());
    if (!Owner) { EndChannel(); return; }

    const FVector Center = Owner->GetActorLocation();
    const float   R2     = ChannelRadius * ChannelRadius;

    // 현재 범위 내 플레이어(자신 포함) 수집
    TSet<TWeakObjectPtr<APTPlayerCharacter>> NowInRange;
    NowInRange.Add(Owner);
    for (TActorIterator<APTPlayerCharacter> It(GetWorld()); It; ++It)
    {
        APTPlayerCharacter* P = *It;
        if (P == Owner) continue;
        if (FVector::DistSquared(P->GetActorLocation(), Center) <= R2)
            NowInRange.Add(P);
    }

    // 이탈자 → 무적 회수
    for (const TWeakObjectPtr<APTPlayerCharacter>& W : ChannelProtected)
        if (!NowInRange.Contains(W))
            if (APTPlayerCharacter* P = W.Get()) P->RemoveInvincibility();

    // 신규 진입자 → 무적 부여
    for (const TWeakObjectPtr<APTPlayerCharacter>& W : NowInRange)
        if (!ChannelProtected.Contains(W))
            if (APTPlayerCharacter* P = W.Get()) P->AddInvincibility();

    ChannelProtected = NowInRange;
}

void UPTPlayerSkillComponent::EndChannel()
{
    if (!bIsChanneling) return;
    bIsChanneling = false;

    if (UWorld* W = GetWorld())
    {
        W->GetTimerManager().ClearTimer(ChannelRefreshTimer);
        W->GetTimerManager().ClearTimer(ChannelMaxTimer);
    }

    // 이 채널이 부여한 무적 전부 회수 (누수 방지)
    for (const TWeakObjectPtr<APTPlayerCharacter>& W : ChannelProtected)
        if (APTPlayerCharacter* P = W.Get()) P->RemoveInvincibility();
    ChannelProtected.Empty();

    Multicast_SetChannelActive(false, ChannelSkillID);

    // 종료 시점에 쿨다운 시작
    if (ChannelCooldown > 0.f && bIsCooldown.IsValidIndex(ChannelSlotIndex))
    {
        bIsCooldown[ChannelSlotIndex] = true;
        const int32 Slot = ChannelSlotIndex;
        if (UWorld* W = GetWorld())
            W->GetTimerManager().SetTimer(CooldownTimers[Slot],
                [this, Slot]() { OnCooldownEnd(Slot); }, ChannelCooldown, false);
        Client_NotifyCooldownStarted(Slot, ChannelCooldown);
    }

    ChannelSlotIndex = INDEX_NONE;
}

void UPTPlayerSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetOwnerRole() == ROLE_Authority) EndChannel();

    Super::EndPlay(EndPlayReason);
}

void UPTPlayerSkillComponent::Server_Dodge_Implementation()
{
    const FPTSkillRow* DodgeData = GetSkillData(DodgeSkillID);
    if (!DodgeData) return;

    UAnimMontage* Montage = DodgeData->SkillMontage.LoadSynchronous();
    if (!Montage) return;

    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    if (PC->CurrentMP < DodgeData->MPCost)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dodge MP 부족 - 현재: %.1f, 필요: %.1f"),
            PC->CurrentMP, DodgeData->MPCost);
        return;
    }

    PC->CurrentMP -= DodgeData->MPCost;

    if (APTBasePlayerState* PS = PC->GetPlayerState<APTBasePlayerState>())
    {
        PS->CurrentMP = PC->CurrentMP;
    }

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

    if (bInvincible)
    {
        PC->AddInvincibility();
    }
    else
    {
        PC->RemoveInvincibility();
    }
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

void UPTPlayerSkillComponent::Server_StartChannelSkill_Implementation(int32 SlotIndex, FName SkillID)
{
if (bIsChanneling) return;

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(GetOwner());
    if (!Owner) return;

    const FPTSkillRow* Row = GetSkillData(SkillID);
    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] 서버: Row 없음 %s"), *SkillID.ToString()); return;
    }

    if (bIsCooldown.IsValidIndex(SlotIndex) && bIsCooldown[SlotIndex]) return;

    if (!IsSkillLearned(SkillID))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] 서버: 미습득 %s"), *SkillID.ToString()); return;
    }
    FText Reason;

    if (!CanUseSkill(*Row, Reason))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ch] 서버: 제한 %s"), *Reason.ToString()); return;
    }

    bIsChanneling    = true;
    ChannelSlotIndex = SlotIndex;
    ChannelSkillID   = SkillID;
    ChannelRadius    = Row->SkillRadius;
    ChannelCooldown  = Row->Cooldown;

    Multicast_SetChannelActive(true, SkillID);
}

void UPTPlayerSkillComponent::Server_EndChannelSkill_Implementation()
{
    EndChannel();
}

void UPTPlayerSkillComponent::Server_ChannelActivate_Implementation()
{
    if (!bIsChanneling) return;

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(GetOwner());
    if (!Owner) return;

    UWorld* W = GetWorld();
    if (!W) return;

    // 노티파이가  한 번만 발동, 타이머 재설정 방지
    if (W->GetTimerManager().IsTimerActive(ChannelRefreshTimer)) return;

    //최대체력의 1%로 (깎기만함 회복 아님)
    Owner->CurrentHP = FMath::Min(Owner->CurrentHP, Owner->MaxHP * 0.01f);
    if (APTBasePlayerState* PS = Owner->GetPlayerState<APTBasePlayerState>())
        PS->CurrentHP = Owner->CurrentHP;

    // 무적 아우라 가동 + 주기 갱신(범위 진입/이탈 반영)
    RefreshChannelProtection();
    W->GetTimerManager().SetTimer(
        ChannelRefreshTimer,
        this,
        &UPTPlayerSkillComponent::RefreshChannelProtection,
        0.2f,
        true
        );

    // 최대 홀드 시간은 "무적 켜진 시점"부터 카운트
    const FPTSkillRow* Row = GetSkillData(ChannelSkillID);
    if (Row && Row->MaxChannelTime > 0.f)
        W->GetTimerManager().SetTimer(
            ChannelMaxTimer,
            [this]
            ()
            { EndChannel(); },
            Row->MaxChannelTime,
            false
            );
}

void UPTPlayerSkillComponent::Multicast_SetChannelActive_Implementation(bool bActive, FName SkillID)
{
    APTPlayerCharacter* PC = Cast<APTPlayerCharacter>(GetOwner());
    if (!PC) return;

    const FPTSkillRow* Row = GetSkillData(SkillID);

    if (bActive)
    {
        if (Row)
        {
            // 몽타주 자체를 루프로 세팅해두면 홀드 동안 유지됨
            if (UAnimMontage* M = Row->SkillMontage.LoadSynchronous())
                PC->PlayAnimMontage(M);

            // 채널 지속 VFX (캐릭터에 부착, 끝날 때 직접 정리)
            if (UNiagaraSystem* Fx = Row->SkillEffect.LoadSynchronous())
                ChannelVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
                    Fx, PC->GetRootComponent(), NAME_None,
                    FVector::ZeroVector, FRotator::ZeroRotator,
                    EAttachLocation::KeepRelativeOffset, false);
        }
    }
    else
    {
        if (Row)
        {
            if (UAnimMontage* M = Row->SkillMontage.LoadSynchronous())
            {
                if (UAnimInstance* Anim = PC->GetMesh() ? PC->GetMesh()->GetAnimInstance() : nullptr)
                {
                    // Loop 무한반복에서 빠져나와 End 섹션 재생
                    Anim->Montage_SetNextSection(TEXT("Loop"), TEXT("End"), M);
                    // (섹션 이름은 몽타주에 실제로 붙인 이름으로)
                }
            }
        }

        if (ChannelVFX)
        {
            ChannelVFX->Deactivate();
            ChannelVFX->DestroyComponent();
            ChannelVFX = nullptr;
        }
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
        const UPTEquipmentComponent* Equip = PC->FindComponentByClass<UPTEquipmentComponent>();
        if (!Equip || !Equip->IsWeaponEquipped())
        {
            OutReason = FText::FromString(TEXT("무기를 장착해야 스킬을 사용할 수 있습니다."));
            return false;
        }

        const EWeaponType EquippedType = Equip->GetEquippedWeaponType();
        if (!Row.IsWeaponAllowed(EquippedType))
        {
            OutReason = FText::FromString(TEXT("이 스킬에 맞는 무기를 장착해야 합니다."));
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
            // 추후 추가
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
