#include "PTAnimNotifyState_SkillDash.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Character/Skill/PTSkillRow.h"

void UPTAnimNotifyState_SkillDash::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float InTotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, InTotalDuration, EventReference);

    APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!OwnerPlayer || !OwnerPlayer->HasAuthority()) return;

    UPTPlayerSkillComponent* SkillComp = OwnerPlayer->SkillComp;
    if (!SkillComp) return;

    FPTSkillRow* SkillData = SkillComp->GetSkillData(SkillComp->GetCurrentSkillID());
    if (!SkillData || SkillData->DashDistance <= 0.f) return;

    StartLocation = OwnerPlayer->GetActorLocation();
    TargetLocation = StartLocation + OwnerPlayer->GetActorForwardVector() * SkillData->DashDistance;
    TargetLocation.Z = StartLocation.Z;
    ElapsedTime = 0.f;
    TotalDuration = InTotalDuration;
}

void UPTAnimNotifyState_SkillDash::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    APTPlayerCharacter* OwnerPlayer = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!OwnerPlayer || !OwnerPlayer->HasAuthority()) return;
    if (TotalDuration <= 0.f) return;

    ElapsedTime += FrameDeltaTime;
    float Alpha = FMath::Clamp(ElapsedTime / TotalDuration, 0.f, 1.f);
    float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);

    FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, EasedAlpha);
    OwnerPlayer->SetActorLocation(NewLocation, true);
}

void UPTAnimNotifyState_SkillDash::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                              const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);
}
