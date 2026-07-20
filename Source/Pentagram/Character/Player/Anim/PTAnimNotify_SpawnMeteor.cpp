#include "PTAnimNotify_SpawnMeteor.h"

#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTPlayerSkillComponent.h"
#include "Character/Skill/PTSkillRow.h"
#include "Character/Skill/PTMeteorActor.h"

void UPTAnimNotify_SpawnMeteor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!Owner || !MeteorClass) return;

    UPTPlayerSkillComponent* SkillComp = Owner->SkillComp;
    FPTSkillRow* SkillData = SkillComp ? SkillComp->GetSkillData(SkillRowName) : nullptr;
    if (!SkillData) return;

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    // MOBA 인디케이터로 지정한 착지 지점 (replicated)
    FVector TargetGround = SkillComp->TargetLocation;

    {
        const FVector TraceStart = TargetGround + FVector(0.f, 0.f, FallHeight);
        const FVector TraceEnd   = TargetGround - FVector(0.f, 0.f, 5000.f);

        FHitResult GroundHit;
        FCollisionQueryParams QParams;
        QParams.AddIgnoredActor(Owner);

        // Visibility 채널로 지형만 (Pawn 무시)
        if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QParams))
        {
            TargetGround = GroundHit.ImpactPoint;
        }
    }

    UNiagaraSystem* ImpactVFX = !ImpactVFXOverride.IsNull()
        ? ImpactVFXOverride.LoadSynchronous()
        : SkillData->SkillEffect.LoadSynchronous();
    USoundBase* ImpactSound = !ImpactSoundOverride.IsNull()
        ? ImpactSoundOverride.LoadSynchronous()
        : SkillData->SkillHitSound.LoadSynchronous();

    FActorSpawnParameters Params;
    Params.Owner = Owner;
    Params.Instigator = Owner->GetInstigator();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APTMeteorActor* Meteor = World->SpawnActor<APTMeteorActor>(
        MeteorClass, TargetGround + FVector(0,0,FallHeight), FRotator::ZeroRotator, Params);
    if (!Meteor) return;

    Meteor->InitMeteor(
        Owner, TargetGround, FallHeight, FallSpeed,
        SkillData->SkillRadius, SkillData->DamageMultiplier,
        Owner->HasAuthority(),            // 서버만 데미지
        ImpactVFX, ImpactSound,
        SkillData->MakeHitInfo(Owner));
}
