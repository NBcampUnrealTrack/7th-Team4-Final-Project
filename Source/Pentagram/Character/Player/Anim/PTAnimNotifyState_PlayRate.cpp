#include "PTAnimNotifyState_PlayRate.h"

void UPTAnimNotifyState_PlayRate::NotifyBegin(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
    if (!AnimInst) return;

    UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();
    if (CurrentMontage)
    {
        AnimInst->Montage_SetPlayRate(CurrentMontage, TargetPlayRate);
    }
}

void UPTAnimNotifyState_PlayRate::NotifyEnd(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
    if (!AnimInst) return;

    UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();
    if (CurrentMontage)
    {
        AnimInst->Montage_SetPlayRate(CurrentMontage, 1.0f);  // 내려치기 구간 PlayRate 복구
    }
}
