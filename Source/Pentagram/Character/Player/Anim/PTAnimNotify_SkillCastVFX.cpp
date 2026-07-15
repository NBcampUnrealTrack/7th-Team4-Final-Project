#include "PTAnimNotify_SkillCastVFX.h"

#include "NiagaraFunctionLibrary.h"
#include "Character/Player/PTPlayerCharacter.h"
#include "Character/Skill/PTSkillRow.h"
#include "Character/Skill/PTPlayerSkillComponent.h"

void UPTAnimNotify_SkillCastVFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APTPlayerCharacter* Owner = Cast<APTPlayerCharacter>(MeshComp->GetOwner());
    if (!Owner) return;

    UPTPlayerSkillComponent* SkillComp = Owner->SkillComp;
    if (!SkillComp) return;

    FPTSkillRow* SkillData = SkillComp->GetSkillData(SkillComp->GetCurrentSkillID());
    if (!SkillData) return;

    FVector Center = SkillComp->TargetLocation;

    UNiagaraSystem* CastVFX = CastVFXOverride.LoadSynchronous();
    if (CastVFX)
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            Owner->GetWorld(), CastVFX, Center, FRotator::ZeroRotator);
}
