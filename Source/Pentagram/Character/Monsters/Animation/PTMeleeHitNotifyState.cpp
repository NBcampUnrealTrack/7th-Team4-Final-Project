#include "Character/Monsters/Animation/PTMeleeHitNotifyState.h"
#include "Character/Monsters/PTBossMonsterCharacter.h"
#include "Character/PTBaseCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Monsters/AI/PTMonsterBlackboardKeys.h"

void UPTMeleeHitNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    APTBossMonsterCharacter* Boss = GetBoss(MeshComp);
    if (!IsValid(Boss) || !Boss->HasAuthority())
    {
        return;
    }

    AActor* Target = GetTarget(Boss);
    if (!IsValid(Target))
    {
        Boss->StopAnimMontage();
        return;
    }

    const float Dist = FVector::Dist(Boss->GetActorLocation(), Target->GetActorLocation());
    if (Dist > StartCheckRadius)
    {
        Boss->StopAnimMontage();
        return;
    }

    Boss->ClearHitActors();

    if (IsValid(MeshComp) && MeshComp->DoesSocketExist(HitSocketName))
    {
        PrevSocketLocations.Add(MeshComp, MeshComp->GetSocketLocation(HitSocketName));
    }
}

void UPTMeleeHitNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    APTBossMonsterCharacter* Boss = GetBoss(MeshComp);
    if (!IsValid(Boss) || !Boss->HasAuthority())
    {
        return;
    }

    AActor* Target = GetTarget(Boss);
    if (!IsValid(Target))
    {
        return;
    }

    FVector ToTarget = Target->GetActorLocation() - Boss->GetActorLocation();
    ToTarget.Z = 0.f;
    ToTarget = ToTarget.GetSafeNormal();

    if (!ToTarget.IsNearlyZero())
    {
        const FRotator NewRotator = FMath::RInterpTo(
            Boss->GetActorRotation(),
            ToTarget.Rotation(),
            FrameDeltaTime,
            HomingStrength);
        Boss->SetActorRotation(NewRotator);
    }

    UWorld* World = Boss->GetWorld();
    if (!IsValid(World) || !IsValid(MeshComp) || !MeshComp->DoesSocketExist(HitSocketName))
    {
        return;
    }

    const FVector CurrentLocation = MeshComp->GetSocketLocation(HitSocketName);
    FVector PrevLocation = CurrentLocation;
    if (FVector* Found = PrevSocketLocations.Find(MeshComp))
    {
        PrevLocation = *Found;
    }

    TArray<FHitResult> HitResults;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Boss);

    World->SweepMultiByChannel(
        HitResults, PrevLocation,
        CurrentLocation, FQuat::Identity,
        ECC_Pawn, FCollisionShape::MakeSphere(HitRadius),
        Params);

    for (const FHitResult& Hit : HitResults)
    {
        APTBaseCharacter* TargetChar = Cast<APTBaseCharacter>(Hit.GetActor());
        if (!IsValid(TargetChar))
        {
            continue;
        }

        if (TargetChar == Boss)
        {
            continue;
        }

        if (TargetChar->IsDead())
        {
            continue;
        }

        TWeakObjectPtr<AActor> WeakTarget = TargetChar;
        if (Boss->IsAlreadyHit(WeakTarget))
        {
            continue;
        }

        Boss->AddHitActor(WeakTarget);

        if (USoundBase* Sound = HitSound.LoadSynchronous())
        {
            Boss->MulticastPlayHitSound(CurrentLocation, Sound);
        }

        FPTHitInfo HitInfo = Boss->GetCurrentMeleeHitInfo();
        HitInfo.HitDirection = (TargetChar->GetActorLocation() - CurrentLocation).GetSafeNormal();
        TargetChar->ApplyDamageWithHit(Boss->GetCurrentMeleeDamage(), Boss, HitInfo);
    }

    PrevSocketLocations.Add(MeshComp, CurrentLocation);
}

void UPTMeleeHitNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    APTBossMonsterCharacter* Boss = GetBoss(MeshComp);
    if (IsValid(Boss))
    {
        Boss->ClearHitActors();
    }

    PrevSocketLocations.Remove(MeshComp);
}

APTBossMonsterCharacter* UPTMeleeHitNotifyState::GetBoss(USkeletalMeshComponent* MeshComp) const
{
    if (!IsValid(MeshComp))
    {
        return nullptr;
    }

    return Cast<APTBossMonsterCharacter>(MeshComp->GetOwner());
}

AActor* UPTMeleeHitNotifyState::GetTarget(APTBossMonsterCharacter* Boss) const
{
    if (!IsValid(Boss))
    {
        return nullptr;
    }

    const AAIController* AIC = Cast<AAIController>(Boss->GetController());
    if (!IsValid(AIC))
    {
        return nullptr;
    }

    const UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (!IsValid(BB))
    {
        return nullptr;
    }

    return Cast<AActor>(BB->GetValueAsObject(PTMonsterBlackboardKeys::TargetActor));
}
