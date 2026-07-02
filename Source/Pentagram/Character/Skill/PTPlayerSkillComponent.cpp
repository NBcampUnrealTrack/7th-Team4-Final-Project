#include "PTPlayerSkillComponent.h"

#include "Character/Player/PTBasePlayerState.h"
#include "Character/PTBaseCharacter.h"
#include "Character/Player/PTEquipmentComponent.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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

    // MP 체크 및 차감
    APTBaseCharacter* Owner = Cast<APTBaseCharacter>(GetOwner());
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill Owner 실패"));
        return;
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
        Multicast_PlaySkillMontageWithOffset(Montage, Effect, Sound, SkillData->SkillOffset, Request.SkillRowName);

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

    const FPTSkillRow* SkillData = GetSkillData(BasicAttackSkillID);
    if (!SkillData || SkillData->ComboMontage.Num() == 0) return;

    if (!SkillData->ComboMontage.IsValidIndex(ComboIndex))
    {
        ComboIndex = 0;
    }

    UAnimMontage* Montage = SkillData->ComboMontage[ComboIndex].LoadSynchronous();
    if (!Montage) return;

    bIsAttacking = true;

    if (APTPlayerCharacter * PlayerChar = Cast<APTPlayerCharacter>(GetOwner()))
    {
        PlayerChar->EnterCombat();
    }

    bCanCombo = false;

    if (PC->IsLocallyControlled())
    {
        PC->PlayAnimMontage(Montage);
    }

    Server_BasicAttack(ComboIndex);

    ComboIndex++;
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
    if (!SkillData || !SkillData->ComboMontage.IsValidIndex(InComboIndex)) return;

    if (UAnimMontage* Montage = SkillData->ComboMontage[InComboIndex].LoadSynchronous())
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

void UPTPlayerSkillComponent::OnCooldownEnd(int32 SlotIndex)
{
    Super::OnCooldownEnd(SlotIndex);

    // UI 쿨다운 종료 델리게이트 발행
    OnSkillCooldownEnd.Broadcast(SlotIndex);
}

void UPTPlayerSkillComponent::Client_NotifyCooldownStarted_Implementation(int32 SlotIndex, float Duration)
{
    OnSkillCooldownStart.Broadcast(SlotIndex, Duration);
}
