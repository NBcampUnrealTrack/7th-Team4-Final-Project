#include "PTAnimNotifyState_SkillBP.h"

#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTPlayerSkillComponent.h"

void UPTAnimNotifyState_SkillBP::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!Owner || !SpawnBPClass) return;

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    FVector SpawnLoc;
    FRotator SpawnRot = Owner->GetActorRotation();

    if (bUseTargetLocation && Owner->SkillComp)
    {
        SpawnLoc = Owner->SkillComp->TargetLocation;
    }
    else
    {
        FVector Offset = Owner->GetActorForwardVector() * SpawnOffset.X
                       + Owner->GetActorRightVector()   * SpawnOffset.Y
                       + FVector(0.f, 0.f, SpawnOffset.Z);
        SpawnLoc = Owner->GetActorLocation() + Offset;
    }

    FActorSpawnParameters Params;
    Params.Owner = Owner;
    Params.Instigator = Owner->GetInstigator();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    SpawnedActor = World->SpawnActor<AActor>(SpawnBPClass, SpawnLoc, SpawnRot, Params);
}

void UPTAnimNotifyState_SkillBP::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (SpawnedActor.IsValid())
    {
        SpawnedActor->Destroy();
        SpawnedActor = nullptr;
    }
}
